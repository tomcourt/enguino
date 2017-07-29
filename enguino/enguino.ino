#include <SPI.h>            // required for ethernet2
#include <Ethernet2.h>
#include <string.h>
#include <avr/pgmspace.h>   // storing strings in Flash to save RAM

// Temporary chang this your local network's IP range for testing
IPAddress ip(192, 168, 0, 111);   // http://192.168.0.111 is link to Enguino

// fictitious MAC address. Only real critera is the first byte's 2 lsb must be 1 (for local) and 0 (for unicast).
byte mac[] = {  0xDE, 0x15, 0x24, 0x33, 0x42, 0x51 };

EthernetServer server(80);    // Port 80 is HTTP
EthernetClient client;

// #define DEBUG                 // checks RAM usage
// #define SIMULATE_SENSORS 3    // number of simulated sensor 'states', use serial to advace state

// sketches don't like typdef's so they are in in this header file instead
#include "egTypes.h"

#include "utility.h"

#define HOBBS_COUNT_INTERVAL (3600/40)    // update hobbs 40 times an hour
bool leanMode;
int peakEGT[4];
bool eeUpdateDirty;
byte hobbsCount = HOBBS_COUNT_INTERVAL/2;  // in order to prevent cumulative hobbs error assume half a hobbs count of engine run time was lost on last shutdown
bool engineRunning;

volatile bool tachDidPulse;
volatile int rpm[8];

// printLED functions for the auxiliary display
#include "printLED.h"

// configuration of sensors and layout of the gauges
#include "config.h"

#include "sensors.h"

// load and save persistant EEPROM data
#include "persist.h"

// Performance 'print' functions to ethernet 'client' (includes flush)
#include "printEthernet.h"


// Implementation for printPrefix and pringGauge
#include "printGauges.h"
#include "printWeb.h"

// Measure thermocouple tempertures in the background
#include "tcTemp.h"

bool dimAux;
bool didHoldKey;
bool didChangeDim;
byte auxPage = 0; 
signed char blinkAux[2]; 



void updateAlerts() {
  for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++) {
    AuxDisplay *a = auxDisplay + i;
    for (byte j=0; j<2; j++) {
      Sensor *s = a->sensor[j];
      byte b = alertState(s, 0);
      if (s->pin & DUAL_BIT)
        b |= alertState(s, 1);
      a->alertState[j] = b;     
    }
  }
}

void checkForAlerts(bool warning) {
  // work backward to higher priority alerts
  for (byte i=N(auxDisplay)-1; i>=AUX_STARTUP_PAGES; i--) {
    AuxDisplay *a = auxDisplay + i;
    bool isInfoPage = a->sensor[0] != a->sensor[1];
    if (warning) { 
      bool b = (a->alertState[1] & WARNING_ANY);
      if (isInfoPage) 
        b = b || (a->alertState[0] & WARNING_ANY);
      if (b) {
        alertStatus = STATUS_WARNING;
        if (a->warning != -1) {
          a->warning = 1;
          auxPage = i;
         }
      }
    }
    else {
      if (!isInfoPage && (a->alertState[1] & CAUTION_ANY)) {    // no caution check on info pages
        alertStatus = STATUS_CAUTION;
        if (a->caution != -1) {
          a->caution = 1;
          auxPage = i;
        }
      }
    }
  }
}

void showAuxPage(byte inx) {
  AuxDisplay *a = auxDisplay+inx;  
  for (byte n=0; n<2; n++) {
 
    if (a->alertState[n] & WARNING_ANY) {
      alertStatus = STATUS_WARNING;
      if (blinkAux[n] == 0) 
         blinkAux[n] = 1;
    }
    else 
      blinkAux[n] = 0; 
    commandLED(n, (blinkAux[n] == 1) ? HT16K33_BLINK_1HZ : HT16K33_BLINK_OFF);  
     
    Sensor *s = a->sensor[n]; 
    int v = scaleValue(s, readSensor(s)); 
    if (n==0 && a->literal[0]) {
      byte t[4];
      memcpy(t,a->literal,4);
      if (a->alertState[n])
        t[3] = (a->alertState[1] & (WARNING_LOW | CAUTION_LOW)) ? LED_L : LED_H;
      printLED(n,t);     
    }
    else if (s->pin & DUAL_BIT)      
      printLEDFuel(scaleValue(s, readSensor(s,0)), scaleValue(s, readSensor(s,1)));    // Show the dual fuel gauge
    else if (s) 
      printLED(n,v, s->decimal);
  }
}

bool ackAlert(byte inx) {
 if (auxDisplay[inx].warning > 0) { 
    auxDisplay[inx].warning = -1;
    auxDisplay[inx].caution = -1;
    return true;
  }
  else if (auxDisplay[inx].caution > 0) {
    auxDisplay[inx].caution = -1;
    return true;
  }
  return false;
}

void ackBlink() {
  if (blinkAux[0] > 0)
    blinkAux[0] = -1;
  if (blinkAux[1] > 0)
    blinkAux[1] = -1;
}

void setup() {
  Serial.begin(9600);
//    while (!Serial) 
//      ; // wait for serial port to connect. Stops here until Serial Monitor is started. Good for debugging setup

  sensorSetup();

  delay(1); // delay to allow LED display chip to startup
  
  eeInit();
  
  tcTempSetup();
  printLEDSetup();
  for (auxPage=0; auxPage<AUX_STARTUP_PAGES; auxPage++) {
    showAuxPage(auxPage);
    delay(2000);
  }
  switchPress = 0;
   
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
}

void loop() {
  if (switchPress > 0) {
    if (switchPress < 8) {
      // Press (acknowedge/next-page)
      // ----------------------------
      if (blinkAux[0] > 0 || blinkAux[1] > 0) {
        ackAlert(auxPage);
        ackBlink();
      }
      else {
        if (ackAlert(auxPage))
          auxPage = AUX_STARTUP_PAGES;
        else {
          auxPage++;
          if (auxPage >= N(auxDisplay))
            auxPage = AUX_STARTUP_PAGES;
        }
      }
    }
    switchPress = 0;
    didChangeDim = false;
    didHoldKey = false;
  }
  if (switchDown >= 24 && !didChangeDim) {
    // Long hold (dim,bright toggle)
    dimAux = !dimAux;
    for (byte line=0; line<2; line++)   
      commandLED(line, dimAux?HT16K33_BRIGHT_MIN:HT16K33_BRIGHT_MAX); 
    didChangeDim = true; 
    didHoldKey = false;
  }
  else if (switchDown >= 8 && !didHoldKey) {
    // Hold (reshow all)
    // -----------------
    // un-ack all and reshow alerts
    blinkAux[0] = blinkAux[1] = 0; 
    for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++) 
      auxDisplay[i].warning = auxDisplay[i].caution = 0;
    auxPage = AUX_STARTUP_PAGES;
    didHoldKey = true;
  }

#if SIMULATE_SENSORS
  if (Serial.read() >= 0) {
    if (++simState >= SIMULATE_SENSORS)
      simState = 0;
  }
#endif

  pollForHttpRequest();
  
  if (eighthSecondTick) {
    eighthSecondTick = false;
    updateADC();

    if (eighthSecondCount == 4 || eighthSecondCount >= 8) {
      if (eighthSecondCount >= 8) { // every second
#if DEBUG
        logValue(minFreeRam,"minFreeRam");
#endif

        updateTach();
           
        engineRunning = isEngineRunning();
  
        if (engineRunning)
          updateHobbs();

        if (eeUpdateDirty) {
          eeUpdateStatus();
          eeUpdateDirty = false;
        }  
        eighthSecondCount -= 8;
      }
      // every half second
      if (engineRunning) {
        alertStatus = STATUS_NORMAL;
        updateAlerts();
        checkForAlerts(false); 
        checkForAlerts(true);  
      }
      else
        alertStatus = STATUS_WARNING;
      showAuxPage(auxPage); 
    }
  }
}

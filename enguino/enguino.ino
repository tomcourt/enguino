#include <SPI.h>            // required for ethernet2
#include <Ethernet2.h>
#include <string.h>
#include <avr/pgmspace.h>   // storing strings in Flash to save RAM

// sketches don't like typdef's so they are in in this header file instead
#include "egTypes.h"

#include "utility.h"

#include "persist.h"

// Digital pin assignments
#define AUX_SWITCH 0
#define TACH_SIGNAL 2

bool leanMode;
int peakEGT[4];
byte hobbsCount = 3600/40;    // update hobbs 40 times an hour
bool engineRunning;

volatile unsigned long lastTachTime;
volatile bool tachDidPulse;
volatile int rpm[8];
volatile byte rpmInx;

#define SIMULATE_SENSORS 1
#include "sensors.h"

// printLED functions for the auxiliary display
#include "printLED.h"

// configuration of sensors and layout of the gauges
#include "config.h"

// Performance 'print' functions to ethernet 'client' (includes flush)
#include "printEthernet.h"


// Implementation for printPrefix and pringGauge
#include "printGauges.h"
#include "printWeb.h"

// Measure thermocouple tempertaures in the background
#include "tcTemp.h"

bool dimAux;
bool didHoldKey;
bool didChangeDim;
byte auxPage = AUX_STARTUP_PAGES; // next page is info page
signed char blinkAux[2]; 

void tachIRQ() {
  unsigned long newTachTime = micros();
  rpm[rpmInx++ & 7] = (60000000L/TACH_DIVIDER) / (newTachTime - lastTachTime);
  lastTachTime = newTachTime;
  tachDidPulse = true;
}

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
//
//  logValue(ARCX(-.004),"x -");
//  logValue(ARCY(-.004),"y -");
//  logValue(ARCX(0),"x 0");
//  logValue(ARCY(0),"y 0");
//  logValue(ARCX(.167),"x .166");
//  logValue(ARCY(.167),"y .166");
//  logValue(ARCX(.9),"x .9");
//  logValue(ARCY(.9),"y .9");
//  logValue(ARCX(1),"x 1");
//  logValue(ARCY(1),"y 1");

  pinMode(AUX_SWITCH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TACH_SIGNAL),tachIRQ,RISING);

  delay(1); // delay to allow LED display chip to startup
  eeInit();  
  tcTempSetup();
  printLEDSetup();
  printLED(0,LED_TEXT(h,o,b,b));
  printLED(1,ee_status.hobbs>>2,1);  
  delay(1000);
  showAuxPage(0);
  delay(1000);
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
    // Hold (acknowedge all, reshow all)
    // ---------------------------------
    if (blinkAux[0] > 0 || blinkAux[1] > 0 || auxDisplay[auxPage].caution > 0 || auxDisplay[auxPage].warning > 0) {
      ackBlink();
      for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++) 
        ackAlert(i);
    }
    else {
      // un-ack all and reshow alerts
      blinkAux[0] = blinkAux[1] = 0; 
      for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++) 
        auxDisplay[i].warning = auxDisplay[i].caution = 0;
    }
    auxPage = AUX_STARTUP_PAGES;
    didHoldKey = true;
  }

#if SIMULATE_SENSORS
  if (Serial.read() >= 0) {
    if (++simState > 3)
      simState = 1;
  }
#endif

  pollForHttpRequest();
  
  if (eighthSecondTick) {
    eighthSecondTick = false;
    updateADC();
    
    if (eighthSecondCount == 4)
      goto halfSecond;
    if (eighthSecondCount >= 8) {
      if (tachDidPulse)
        tachDidPulse = false;
      else
        memset(rpm, 0, sizeof(rpm));
        
      engineRunning = true
#if RUN_VOLT
        || scaleValue(&voltS, readSensor(&voltS)) > RUN_VOLT
#endif
#if RUN_OILP
       || scaleValue(&oilpS, readSensor(&oilpS)) > RUN_OILP
#endif
#if RUN_TACH
       || scaleValue(&tachS, readSensor(&tachS)) > RUN_TACH
#endif
        ;
  
      if (engineRunning) {
        if (--hobbsCount == 0) {
          if (++(ee_status.hobbs) > 39999) {
            ee_status.hobbs = 0;
            ee_status.hobbs1k++;
          }
          eeUpdateStatus();
          hobbsCount = 90;
        }
      }
      
      eighthSecondCount -= 8;
  
halfSecond:
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

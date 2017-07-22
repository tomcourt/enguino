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

// #define SIMULATE_SENSORS 1
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

bool isLow(Sensor *s, int v) {
  return v < s->lowAlarm || v < s->lowAlert;
}

bool isHigh(Sensor *s, int v) {
  return v > s->highAlarm || v > s->highAlert;
}

bool isCautionOrAlarm(Sensor *s, int v) {
  return isLow(s, v) || isHigh(s, v);
}

bool isCaution(Sensor *s, int v) {
  return v < s->lowAlert || v > s->highAlert; 
}

bool isAlarm(Sensor *s, int v) {
  return v < s->lowAlarm || v > s->highAlarm; 
}

bool isAlarm(Sensor *s) {
  if (s == &fuellS)
    return isAlarm(s,scaleValue(s, readSensor(s))) || isAlarm(s,scaleValue(s, readSensor(s,1)));
  else
    return isAlarm(s,scaleValue(s, readSensor(s)));
}

bool isCaution(Sensor *s) {
  return isCaution(s,scaleValue(s, readSensor(s)));
}

void cautionAlarmCheck(bool alarm) {
  // work backward to higher priority alerts
  for (byte i=N(auxDisplay)-1; i>=AUX_STARTUP_PAGES; i--) {
    Sensor *s1 = auxDisplay[i].sensor[1];
    Sensor *s0 = auxDisplay[i].sensor[0];
    if (alarm) { 
      bool a = isAlarm(s1);
      if (s1 != s0) 
        a = a || isAlarm(s0);
      if (a) {
        alarmStatus = STATUS_ALARM;
        if (auxDisplay[i].alarm != -1) {
          auxDisplay[i].alarm = 1;
          auxPage = i;
         }
      }
    }
    else {
      if (s1 == s0 && isCaution(s1)) {    // no caution check on info pages
        alarmStatus = STATUS_CAUTION;
        if (auxDisplay[i].caution != -1) {
          auxDisplay[i].caution = 1;
          auxPage = i;
        }
      }
    }
  }
}

void showAuxPage(byte inx) {
  AuxDisplay *a = auxDisplay+inx;  
  for (byte n=0; n<2; n++) {
    Sensor *s = a->sensor[n]; 

    if (isAlarm(s)) {
      alarmStatus = STATUS_ALARM;
      if (blinkAux[n] == 0) 
         blinkAux[n] = 1;
    }
    else 
      blinkAux[n] = 0; 
    commandLED(n, (blinkAux[n] == 1) ? HT16K33_BLINK_1HZ : HT16K33_BLINK_OFF);  
     
    int v = scaleValue(s, readSensor(s)); 
    if (n==0 && a->literal[0]) {
      byte t[4];
      memcpy(t,a->literal,4);
      if (s) {
        if (isCautionOrAlarm(s, v))
          t[3] = isLow(s,v) ? LED_L : LED_H;
      }
      printLED(n,t);     
    }
    else if (s == &fuellS)      
      printLEDFuel(scaleValue(s, readSensor(s,0)), scaleValue(s, readSensor(s,1)));    // Show the dual fuel gauge
    else if (s) 
      printLED(n,v, s->decimal);
  }
}

bool ackAlert(byte inx) {
 if (auxDisplay[inx].alarm > 0) { 
    auxDisplay[inx].alarm = -1;
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
    if (blinkAux[0] > 0 || blinkAux[1] > 0 || auxDisplay[auxPage].caution > 0 || auxDisplay[auxPage].alarm > 0) {
      ackBlink();
      for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++) 
        ackAlert(i);
    }
    else {
      // un-ack all and reshow alarms
      blinkAux[0] = blinkAux[1] = 0; 
      for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++) 
        auxDisplay[i].alarm = auxDisplay[i].caution = 0;
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
        alarmStatus = STATUS_NORMAL;
        cautionAlarmCheck(false); 
        cautionAlarmCheck(true);  
      }
      else
        alarmStatus = STATUS_ALARM;
      showAuxPage(auxPage);  
    }
  }
}

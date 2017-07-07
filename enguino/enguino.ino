#include <SPI.h>            // required for ethernet2
#include <Ethernet2.h>
#include <string.h>
#include <avr/pgmspace.h>   // storing strings in Flash to save RAM

// sketches don't like typdef's so they are in in this header file instead
#include "egTypes.h"

#include "utility.h"

#include "persist.h"

// Update the first 3 numbers of this IP address to your local network
// for testing but restore it to (192, 168, 0, 111) when finished.
IPAddress ip(192, 168, 0, 111);

// A made up MAC address. Only real critera is the first bytes 2 lsb must be 1 (for local) and 0 (for unicast).
byte mac[] = {  0xDE, 0x15, 0x24, 0x33, 0x42, 0x51 };

int readSensor(const Sensor *s, byte n = 0);

// #define RANDOM_SENSORS 1

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP)

EthernetServer server(80);
EthernetClient client;

bool leanMode;
int peakEGT[4];
byte hobbsCount = 90;
bool engineRunning;

volatile unsigned long lastTachTime;
volatile bool tachDidPulse;
volatile int rpm[4];
volatile byte rpmP;

bool dim;
bool didKeyDown;
byte auxScreen = 2;

// printLED functions for the auxiliary display
#include "printLED.h"

// configuration of sensors and layout of the gauges
#include "config.h"

// Performance 'print' functions to ethernet 'client' (includes flush)
#include "printEthernet.h"

#include "printWeb.h"

// Implementation for printPrefix and pringGauge
#include "printGauges.h"

// Measure thermocouple tempertaures in the background
#include "tcTemp.h"



InterpolateTable thermistor = {
  64, 32,
  (byte []) { 
    3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5,
    5, 5, 5, 6, 6, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4 
  },
  (int []) { 
    1538, 1480, 1428, 1382, 1341, 1303, 1269, 1208,
    1154, 1107, 1064, 1026,  990,  957,  897,  844,
     796,  752,  711,  635,  564,  431,  363,  291,
     252,  210,  164,  112,   83,   50,   13,  -30 
  },
};

InterpolateTable r240to33 = {
  48,  28,
  (byte []){ 
    5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
  },
  (int []){ 
    1105, 1064, 1019, 972, 921, 894, 866,
     837,  806,  775, 742, 707, 671, 634,
     595,  553,  510, 465, 417, 367, 314,
     258,  199,  137,  70,   0, -75, -155
  },
 };

int readSensor(const Sensor *s, byte n = 0) {
  int p = s->pin + n;
  
  #ifdef RANDOM_SENSORS
    if (p < 16)
      return rand() & 0x3ff;        
    if (p < 20)                       
      return rand() & 0x7ff;          // CHT
    if (p < 24)
      return rand() & 0x7ff + 4000;   // EGT 
    return FAULT;
    
  #else
    int v;
    if (p < 0)
      return FAULT;
     
    int t = s->type;
    int toF = 0;
    if (p == 15) {
      // RPM's are occasionaly screwed up because of IRQ latency.
      // Throw out highest and lowest and average the middle 2
      noInterrupts();
      int r[4];
      memcpy(r, rpm, sizeof(rpm));
      interrupts();
      sort(r, sizeof(r)/sizeof(int));
      v = (r[1]+r[2])>>1; 
    }
    else if (p < 16) { 
      v = analogRead(p);
      if (t == st_r240to33)
        v = interpolate(&r240to33, v);
      else if (t == st_thermistorC || t == st_thermistorF) {
        v = interpolate(&thermistor, v);
        if (t == st_thermistorF)
          toF = 32 * 10;
      }
    }
    else if (p < 24) {     
      noInterrupts();
      v = tcTemp[p-16];
      interrupts();
      if (t == st_j_type_tcC || t == st_j_type_tcF)
       v = int(multiply(v - tcTemp[8], 25599) >> 15) + tcTemp[8]; 
      if (t == st_k_type_tcF || t == st_j_type_tcF)
        toF = 32 * 4;
    }
    if (toF && v != FAULT) 
      v = (v*9)/5 + toF;
    return v;     
  #endif
}


void serveUpWebPage(char url, char var, word num) {
  switch(url) {  
    case '?':     // lean/cancel/ button pressed or return from setup
      switch (var) {
        case 'l':     // lean mode
          leanMode = !leanMode;
          if (leanMode)
            memset(peakEGT,0,sizeof(peakEGT));
          break;
        case 'a':   // add fuel
          ee_status.fuel += num<<2;
          if (ee_status.fuel > ee_settings.fullFuel)
            ee_status.fuel = ee_settings.fullFuel;
          goto eeStatus;
        case 'h':   // set hobbs
          ee_status.hobbs1k = 0;
          while (num >= 10000) {
            num -= 10000;
            (ee_status.hobbs1k)++;
          }
          ee_status.hobbs = num<<2; 
eeStatus:
          eeUpdateStatus();
          break;
        case 'f':   // set full fuel
          ee_settings.fullFuel = num<<2;
          goto eeSettings;
        case 'k':   // set k-factor
          ee_settings.kFactor = num;
eeSettings:
          eeUpdateSettings();
          break;
       }
       // fallthru to static page
    case ' ':     // static webpage
      printHomePage();
      break;
    case 's':      // setup page
     printSetupPage();
     break;
   case 'd':     // dynamic webpage   
      for (int i=0; i<N(gauges); i++)
        printGauge(gauges+i);
      break;
  }
}


void tachIRQ() {
  unsigned long newTachTime = micros();
  rpm[rpmP++ & 3] = (60000000L/24) / (newTachTime - lastTachTime);
  lastTachTime = newTachTime;
  tachDidPulse = true;
}

bool isLow(Sensor *s, int v) {
  return v < s->lowAlarm || v < s->lowAlert;
}

bool isHigh(Sensor *s, int v) {
  return v > s->highAlarm || v > s->highAlert;
}

bool isAlarm(Sensor *s, int v) {
  return v < s->lowAlarm || v > s->highAlarm; 
}

void auxDisplay(byte i) {
  int v;
  Sensor *s = 0;
  for (signed char n=1; n>=0; n--) {
    AuxDisplay *a = auxdisplay+(i+n); 
    if (a->sensor.zero == 0) {
      commandLED(n, HT16K33_BLINK_OFF);
      s = a->sensor.s;
      if (s == &flS) {
        printLEDFuel(scaleValue(s, readSensor(s,0)), scaleValue(s, readSensor(s,1)));    // Show the dual fuel gauge
        s = 0;    // fuel gauge stands alone, doesn't have label to blink
      }
      else {
        v = scaleValue(s, readSensor(s));
        printLED(n,v, s->decimal);
      }
    } 
    else {
      byte t[4];
      memcpy(t,a->literal,4);
      if (s) {
        if (isLow(s, v) || isHigh(s, v)) {
          if (t[1] == 0) {
            t[1] = t[2];
            t[2] = t[3];
            t[3] = LED__;  
          }
          t[0] = t[1];
          t[1] = t[2];
          t[2] = t[3];
          t[3] = isLow(s,v) ? LED_L : LED_H;
        }
        if (isAlarm(s, v)) {
          commandLED(n, HT16K33_BLINK_1HZ);
          goto blink;
        }
      }
      commandLED(n, HT16K33_BLINK_OFF);
blink:
      printLED(n,t);
    }
  }
}



void setup() {
//  Serial.begin(9600);
//    while (!Serial) 
//      ; // wait for serial port to connect. Stops here until Serial Monitor is started. Good for debugging setup

  eeInit();  
  tcTempSetup();
  // setup LED last to allow 1mS for HT16K33
  printLEDSetup();
  printLED(0,LED_TEXT(h,o,b,b));
  printLED(1,ee_status.hobbs>>2,1);  
  delay(1000);
  auxDisplay(0);
  delay(1000);
  
  attachInterrupt(digitalPinToInterrupt(2),tachIRQ,RISING);

  pinMode(0, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(0),switchIRQ,FALLING);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
}



void loop() {
  // listen for incoming clients
  if (switchPress) {
    if (!didKeyDown) {
      auxScreen += 2;
      if (auxScreen >= sizeof(auxdisplay)/sizeof(auxdisplay[0]))
        auxScreen = 2;
    }
    switchPress = 0;
    didKeyDown = false;
  }
  if (switchDown >= 32) {
    if (!didKeyDown)
      dim = !dim;
      for (byte line=0; line<2; line++)   
        commandLED(line, dim?HT16K33_BRIGHT_MIN:HT16K33_BRIGHT_MAX);  
      didKeyDown = true;
  }

  client = server.available();
  if (client) {
    // an http request ends with a blank line
    char lastToken = 0;
    const char *request = "GET /?x= &n=";
    char *state = request;
    char url = 0;
    char var = 0;
    word num = 0;
    while (client.connected()) {
      if (client.available()) {
        char token = client.read();
        if (token == '\r')
          continue;               // ignore '/r'
        // Serial.print(token);          
        // two newlines in a row is the end of the request
        if (token == '\n' && lastToken == '\n') {
          // send a standard http response header
          print_P(F(
            "HTTP/1.1 200 OK\n"
            "Content-Type: text/html\n"
            "Connection: close\n"     // the connection will be closed after completion of the response
            "\n"
          ));
          serveUpWebPage(url, var, num);
          flush();   
          break;
        }
        lastToken = token;
        
        if (state != 0) {
          switch (*state) {
            case '\0':
              if (token >= '0' && token <= '9') {
                num = num*10 + token - '0';
                continue;       // hold state
              }
              break;
            case '?':
              url = token;      // either a ? or the url
              break;
            case ' ':
              var = token;
              state++;          // advance state (falling thru would complete request which we don't want to do)
              continue;           
          }
          if (token == *state)
            state++;            // advance state
          else if (url)
            state = 0;          // completed getting the request because other than the templated query
          else
            state = request;    // reset looking for request if we haven't gotten to the '?'
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }

  if (eighthSecondCount == 4)
    goto halfSecond;
  if (eighthSecondCount >= 8) {
    if (tachDidPulse)
      tachDidPulse = false;
    else
      memset(rpm, 0, sizeof(rpm));
    engineRunning = scaleValue(&vtS, readSensor(&vtS)) > 130; // greater than 13.0 volts means engine is running

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
    auxDisplay(auxScreen);  
  }
}

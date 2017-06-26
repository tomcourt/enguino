#include <SPI.h>            // required for ethernet2
#include <Ethernet2.h>
#include <string.h>
#include <avr/pgmspace.h>   // storing strings in Flash to save RAM

// sketches don't like typdef's so they are in in this header file instead
#include "egTypes.h"

#include "utility.h"

// configuration of sensors and layout of the gauges
#include "config.h"

// A made up MAC address. Only real critera is the first bytes 2 lsb must be 1 (for local) and 0 (for unicast).
byte mac[] = {  0xDE, 0x15, 0x24, 0x33, 0x42, 0x51 };

// Update the first 3 numbers of this IP address to your local network
// for testing but restore it to (192, 168, 0, 111) when finished.
IPAddress ip(192, 168, 0, 111);

// #define RANDOM_SENSORS 1

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP)

EthernetServer server(80);
EthernetClient client;

int readGauge(const Gauge *g, byte n = 0);

// Performance 'print' functions to ethernet 'client' (includes flush)
#include "printEthernet.h"

// Implementation for printPrefix and pringGauge
#include "printGauges.h"

// printLED functions for the auxiliary display
#include "printLED.h"

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

int readGauge(const Gauge *g, byte n = 0) {
  int p = g->pin + n;
  
  #ifdef RANDOM_SENSORS
    if (p < 16)
      return rand() & 0x3ff;        
    if (p < 20)                     // CHT
      return rand() & 0x1ff;
    if (p < 24)
      return rand() & 0x1ff + 1000;   // EGT 
    return FAULT;
    
  #else
    int v;
    if (p < 0)
      return FAULT;
     
    if (p < 16) { 
      v = analogRead(p);
      int t = g->sensor->type;
      if (t == st_r240to33)
        v = interpolate(&r240to33, v);
      else if (t == st_thermistor)
        v = interpolate(&thermistor, v);
   }
   else if (p < 24) {     
      noInterrupts();
      v = tcTemp[p-16];
      interrupts();
     }
    return v;     
  #endif
}


void serveUpWebPage(char url) {
// unsigned long start = millis();         
  switch(url) {  
    case ' ':     // static webpage
      printPrefix();
//    logTime(start, "Load page /");      
      break;
    case 'd':     // dynamic webpage   
      for (int i=0; i<N(gauges); i++)
        printGauge(gauges+i);
//   logTime(start, "Load page /d");      
     break;
  }
}



void setup() {
  Serial.begin(9600);
//  while (!Serial) 
//    ; // wait for serial port to connect. Stops here until Serial Monitor is started. Good for debugging setup

  tcTempSetup();
  printLEDSetup();
  printLED(0,LED_TEXT(h,o,b,b));
  printLED(1,1234,1);   // replace this with a value read from EEPROM
  delay(1000);    // replace this with LED self test sequence

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
}



void loop() {
   // listen for incoming clients
  client = server.available();
  if (client) {
    // an http request ends with a blank line
    char lastToken = 0;
    const char *request = "GET /";
    char *state = request;
    char url = ' ';
    while (client.connected()) {
      if (client.available()) {
        char token = client.read();
        if (token == '\r')
          continue;               // ignore '/r'
        if (*state == 0) {
          url = token;            // completed match of 'request', save webpage name
          state = request;
        }
          
        // two newlines in a row is the end of the request
        if (token == '\n' && lastToken == '\n') {
          // send a standard http response header
          print_P(F(
            "HTTP/1.1 200 OK\n"
            "Content-Type: text/html\n"
            "Connection: close\n"     // the connection will be closed after completion of the response
            "\n"
          ));
          serveUpWebPage(url);
          flush();   
          break;
        }
        if (token && token == *state)
          *state ++;          // matches next character in a 'request', check next character
        else
          state = request;    // doesn't match next character, start from beginning

        lastToken = token;
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

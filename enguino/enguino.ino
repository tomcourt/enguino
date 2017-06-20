#include <SPI.h>            // required for ethernet2
#include <Ethernet2.h>
#include <string.h>
#include <avr/pgmspace.h>   // storing strings in Flash to save RAM

// sketches don't like typdef's so they are in in this header file instead
#include "egTypes.h"

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

int readPin(int p);

#define OPEN  -32767
#define SHORT -32768

// Performance 'print' functions to ethernet 'client' (includes flush)
#include "printEthernet.h"

// Implementation for printPrefix and pringGauge
#include "printGauges.h"

#include "printAux.h"

// Measure thermocouple tempertaures in the background
#include "tcTemp.h"


int readPin(int p) {
#ifdef RANDOM_SENSORS

  if (p < 16)
    return rand() & 0x3ff;
  if (p < 20)
    return rand() & 0x3ff;
  return rand() & 0x1ff;
#else

  if (p < 16)
    return analogRead(p);
  noInterrupts();
  int t = tcTemp[p-16];
  interrupts();
   return t;
#endif
}





void serveUpWebPage(char url) {
   unsigned long start = millis();         
   switch(url) {  
    case ' ':
      printPrefix();
      break;
    case 'd':    
      for (int i=0; i<N(gauges); i++)
        printGauge(gauges+i);
      flush();
      break;

  }
//  Serial.print("Load page /");
//  Serial.print(url);
//  Serial.print(' ');
//  Serial.print(int(millis()-start));
//  Serial.print("ms\n");
}



void setup() {
//  Serial.begin(9600);
//  while (!Serial) 
//    ; // wait for serial port to connect. Stops code until Serial Monitor is started. Good for debugging setup

  tcTempSetup();
  printLEDSetup();
  printLED(0,LED_TEXT(h,o,b,b));
  printLED(1,readPin(0),0);   // replace this with a scaled sensor value
  delay(1000);    // replace this with LED self test sequence

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
//  Serial.print("server is at ");
//  Serial.println(Ethernet.localIP());
}



void loop() {
  const char *request = "GET /";
   // listen for incoming clients
  client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    char *state = request;
    char url = ' ';
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\r')
          continue;
        if (*state == 0) {
          url = c;
          state = request;
        }
          
        // Need to look for GET /x, where x is the webpage being loaded (or ' ' for the main page) !!!! 
        // ignore '/r', walk through string pointing to "GET /" called state, when *state == 0 then
        // use the next character to figure out which web page to load.
        // Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          // client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          serveUpWebPage(url);   
          break;
        }
        if (c && c == *state)
          *state ++;
        else
          state = request;

        currentLineIsBlank = (c == '\n');
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

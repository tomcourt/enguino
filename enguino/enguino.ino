
// Copyright 2017, Thomas Court
//
//  This file is part of Enguino.
//  
//  Enguino is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  Enguino is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with Enguino.  If not, see <http://www.gnu.org/licenses/>.

#include <SPI.h>            // required for ethernet2 
#include <Ethernet2.h>

EthernetServer server(80);    // Port 80 is HTTP
EthernetClient client;

#include <EEPROM.h>         // required for persit.h
#include <string.h>
#include <avr/pgmspace.h>   // storing strings in Flash to save RAM

// Temporary change this your local network's IP range for testing
IPAddress ip(192, 168, 0, 111);   // http://192.168.0.111 is link to Enguino

// fictitious MAC address. Only real critera is the first byte's 2 lsb must be 1 (for local) and 0 (for unicast).
byte mac[] = {  0xDE, 0x15, 0x24, 0x33, 0x42, 0x51 };

// #define DEBUG_RAM_USE         // checks maximum RAM usage
// #define CALIBRATION_TEST      // show calibration values instead of fuel flow/lean info panel
// #define SIMULATE_SENSORS 3    // number of simulated sensor 'states', press enter in serial monitor to advace state
// #define BOUNDING_BOX          // shows a box around each instrumment and around the viewable area of the page. Use to help arrange gauges.

// sketches don't like typdef's so they are in in this header file instead
#include "egTypes.h"

bool eeUpdateDirty;
EESettings ee_settings;
EEStatus ee_status;

bool engineRunning;

#include "utility.h"

// printLED functions for the auxiliary display
#include "printAux.h"

// configuration of sensors and layout of the gauges
#include "config.h"

// read sensors
#include "sensors.h"

// load and save persistant EEPROM data
#include "persist.h"

// Performance 'print' functions to ethernet 'client' (includes flush)
#include "printEthernet.h"

// Implement the web pages
#include "printGauges.h"
#include "printWeb.h"

// Measure thermocouple tempertures in the background (also timed background activites)
#include "tcTemp.h"

// User interface for aux display (and alerting)
#include "controlAux.h"



void setup() {
  Serial.begin(9600);
//    while (!Serial) 
//      ; // wait for serial port to connect. Stops here until Serial Monitor is started. Good for debugging setup

  sensorSetup();
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();

  eeInit();
  
  tcTempSetup();

  // init ADC values for battery voltage
  for(byte i=0; i<4; i++)
    updateADC();        

  // 1ms needed to delay to allow LED display chip to startup
  printLEDSetup();
  
  for (auxPage=0; auxPage<AUX_STARTUP_PAGES; auxPage++) {
    showAuxPage();
    delay(2000);
  }
 }



void loop() {  
#if SIMULATE_SENSORS
  if (Serial.read() >= 0) {
    if (++simState >= SIMULATE_SENSORS)
      simState = 0;
    Serial.print("test-");
    Serial.println(simState);
  }
#endif

  pollForHttpRequest();
   
  if (eighthSecond) {    
    eighthSecond = false;   // this can fall behind due to a long event like responding to a http request
    
    updateADC();

    checkSwitches();

    // every half second
    // -----------------
    if (halfSecond >= 4) {  
      halfSecond -= 4;
    
      updateRPM();
      engineRunning = isEngineRunning();

      updateAlerts();

      if (engineRunning) {
        masterAlertStatus = STATUS_NORMAL;
        checkForAlerts();  
      }
      else
        masterAlertStatus = STATUS_WARNING;
        
      showAuxPage(); 

      // every second
      // ------------
      if (wholeSecond >= 8) { 
        wholeSecond -= 8;

#if DEBUG_RAM_USE
        logValue(minFreeRam,"minFreeRam");
#endif
        if (engineRunning)
          updateHobbs();

        if (eeUpdateDirty) {
          eeUpdateStatus();
          eeUpdateDirty = false;
        }  
      }
    }
  }
}

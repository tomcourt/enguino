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



// Create a web server for testing by doing the following:
// Click on simEnguino in box next to run stop, edit scheme, options, working directory: /Library/WebServer/Documents
// Type this in termainal to open up web server:    sudo chmod a+rw /Library/WebServer/Documents
// Type this in terminal to start web server:       sudo apachectl start
// In browser type:                                 http://localhost

// XCode will lock up the second time you try to run an app in a terminal window
// To avoid this, quit terminal after stopping the simulator



#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

int kbhit(void) {
    static bool initflag = false;
    static const int STDIN = 0;

    if (!initflag) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initflag = true;
    }

    int nbbytes;
    ioctl(STDIN, FIONREAD, &nbbytes);  // 0 is STDIN
    return nbbytes;
}

#include "wiring.h"

#include "i2cLED.h"








#define SIMULATE_SENSORS 3    // number of simulated sensor 'states', press enter in serial monitor to advace state
// #define BOUNDING_BOX       // shows a box around each instrumment and around the viewable area of the page. Use to help arrange gauges.



// sketches don't like typdef's so they are in in this header file instead
#include "../../enguino/egTypes.h"

bool eeUpdateDirty;
EESettings ee_settings;
EEStatus ee_status;

bool engineRunning;

#include "../../enguino/utility.h"

// printLED functions for the auxiliary display
#include "../../enguino/printAux.h"

// configuration of sensors and layout of the gauges
#include "../../enguino/config.h"

// read sensors
#include "../../enguino/sensors.h"

// load and save persistant EEPROM data
#include "../../enguino/persist.h"

// Performance 'print' functions to ethernet 'client' (includes flush)
#include "../../enguino/printEthernet.h"

// Implement the web pages
#include "../../enguino/printGauges.h"
#include "../../enguino/printWeb.h"

// User interface for aux display (and alerting)
#include "../../enguino/controlAux.h"





int main(int argc, const char * argv[]) {
	printf("\33[2J");	// clear screen
	
	client.fp = fopen("index.html", "w");
	printHomePage();
	flush();
	fclose(client.fp);
	
	for (auxPage=0; auxPage<AUX_STARTUP_PAGES; auxPage++) {
		printf("\33[H");	// home
		showAuxPage();
		delay(2000);
	}

	switchPress = 0;   

	for (;;) {
		client.fp = fopen("d.html", "w");
		for (byte i=0; i<N(gauges); i++)
			printGauge(gauges+i);
		flush();
		fclose(client.fp);
		
		if (kbhit()) {
			printf("\33[2J");	// clear screen
			int ch = getchar();
			if (ch == 'a')
				buttonPress();
			if (ch == 's')
				buttonHold();
#if SIMULATE_SENSORS
			if (ch == 'S')
				if (++simState >= SIMULATE_SENSORS)
					simState = 0;
#endif
			checkAuxSwitch();	// for the timeout return to main screen
		}
		
		if (engineRunning) {
			alertStatus = STATUS_NORMAL;
			updateAlerts();
			checkForAlerts(false); 
			checkForAlerts(true);  
		}
		else
			alertStatus = STATUS_WARNING;
		
		printf("\33[H");	// home
		showAuxPage();
		printf("a)ck, s)how, S)tate?");
		
		updateTach();
		engineRunning = isEngineRunning();

        if (engineRunning)
			updateHobbs();

        if (eeUpdateDirty) {
			eeUpdateStatus();
			eeUpdateDirty = false;
        }
		
		delay(500);
	}
}

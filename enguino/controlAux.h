// Copyright 2017, Thomas Court
//
//  Implements the user interface/alerting system for the Aux Display.
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



byte auxPage = 0;
signed char blinkAux; // -1=active,acknowledged, 1=active,blinking
bool dimAux;

void changePage(byte page) {
  if (page != auxPage) {
    blinkAux = 0;
    auxPage = page;
  }
}


void updateAlerts() {
  for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++) {
    AuxDisplay *a = auxDisplay + i;
    for (byte j=0; j<2; j++) {
      const Sensor *s = a->sensor[j];
      byte b = alertStateNow(s, 0);
      if (s->pin & DUAL_BIT)
        b |= alertStateNow(s, 1);
      a->alertState[j] = b;
    }
  }
}


void checkForAlerts() {
  // work backward to higher priority alerts
  for (byte i=N(auxDisplay)-1; i>=AUX_STARTUP_PAGES; i--) {
    AuxDisplay *a = auxDisplay + i;
    bool isInfoPage = (a->sensor[0] != a->sensor[1]);
    bool b = (a->alertState[1] & WARNING_ANY);
    if (isInfoPage)
      b = (b || (a->alertState[0] & WARNING_ANY));
    if (b) {
      masterAlertStatus = STATUS_WARNING;
      if (a->warning != -1) {
        a->warning = 1;
        changePage(i);
      }
    }
  }

  if (masterAlertStatus != STATUS_WARNING) {
    // work backward to higher priority alerts
    for (byte i=N(auxDisplay)-1; i>=AUX_STARTUP_PAGES; i--) {
      AuxDisplay *a = auxDisplay + i;
      bool isInfoPage = (a->sensor[0] != a->sensor[1]);
      bool b = (a->alertState[1] & CAUTION_ANY);
      if (isInfoPage)
        b = (b || (a->alertState[0] & CAUTION_ANY));
      if (b) 
        masterAlertStatus = STATUS_CAUTION;
    }
  }
}


void showAuxPage() {
  AuxDisplay *a = auxDisplay + auxPage;
  bool ack = (blinkAux == -1);
  blinkAux = 0;
  alertStatus = STATUS_NORMAL;
  for (byte n=0; n<2; n++) {
    if (a->alertState[n] & WARNING_ANY) {
      alertStatus = STATUS_WARNING;
      blinkAux = (ack ? -1 : 1);
    }
    else if ((a->alertState[n] & CAUTION_ANY) && alertStatus == STATUS_NORMAL)
      alertStatus = STATUS_CAUTION;
 }
  commandLED((blinkAux == 1) ? HT16K33_BLINK_1HZ : HT16K33_BLINK_OFF);

  prepareLED();
  byte line = 0;
  for (byte n=0; n<2; n++) {
    const Sensor *s = a->sensor[n];
    if (n==0 && a->literal[0]) {
      byte t[4];
      memcpy(t,a->literal,4);
      if (a->alertState[0] & (WARNING_ANY|CAUTION_ANY))
        t[3] = ((a->alertState[0] & (WARNING_LOW | CAUTION_LOW)) ? LED_L : LED_H);
      printLED(line,t);
    }
    else if (s->pin & DUAL_BIT)
      printLEDFuel(line,scaleValue(s, readSensor(s,0)), scaleValue(s, readSensor(s,1)));    // Show the dual fuel gauge
    else if (s) {
	    short v = scaleValue(s, readSensor(s));
      printLED(line, v, s->decimal);
	  }
    line += BOTTOM_LINE;
  }
  writeLED();
}


bool ackAlert() {
 if (auxDisplay[auxPage].warning > 0) {
    auxDisplay[auxPage].warning = -1;
    auxDisplay[auxPage].caution = -1;
    return true;
  }
  else if (auxDisplay[auxPage].caution > 0) {
    auxDisplay[auxPage].caution = -1;
    return true;
  }
  return false;
}


bool ackBlink() {
  bool ack = false;
  if (blinkAux > 0) {
    blinkAux = -1;
    ack = true;
  }
  if (blinkAux > 0) {
    blinkAux = -1;
    ack = true;
  }
  return ack;
}


inline void checkSwitches() {
  switch (readKeys()) {
    case LAST_BIT | NEXT_BIT:
      dimAux = !dimAux;
      commandLED(dimAux?HT16K33_BRIGHT_MIN:HT16K33_BRIGHT_MAX);
      changePage(AUX_STARTUP_PAGES);
      break;
    case NEXT_BIT:
      if (auxPage >= N(auxDisplay)-1) 
        changePage(AUX_STARTUP_PAGES);
      else
        changePage(auxPage+1);
      break;
    case ACK_BIT:
      if (ackBlink())
        ackAlert();
      changePage(AUX_STARTUP_PAGES);
      break;
    case LAST_BIT:
      if (auxPage <= AUX_STARTUP_PAGES) 
        changePage(N(auxDisplay)-1);
      else
        changePage(auxPage-1);
      break;
  }
//  if (displayTimeout > SHOW_DEFAULT_AUX_PAGE_TIMEOUT*8) {
//    if (auxDisplay[auxPage].warning <=0 && auxDisplay[auxPage].caution <= 0)
//      changePage(AUX_STARTUP_PAGES);
//  }
}

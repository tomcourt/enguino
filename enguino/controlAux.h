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
signed char blinkAux[2];
bool dimAux;
bool didHoldKey;
bool didChangeDim;



void updateAlerts() {
  for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++) {
    AuxDisplay *a = auxDisplay + i;
    for (byte j=0; j<2; j++) {
      const Sensor *s = a->sensor[j];
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



void showAuxPage() {
  AuxDisplay *a = auxDisplay + auxPage;
  for (byte n=0; n<2; n++) {
    if (a->alertState[n] & WARNING_ANY) {
      alertStatus = STATUS_WARNING;
      if (blinkAux[n] == 0)
         blinkAux[n] = 1;
    }
    else
      blinkAux[n] = 0;
    commandLED(n, (blinkAux[n] == 1) ? HT16K33_BLINK_1HZ : HT16K33_BLINK_OFF);

    const Sensor *s = a->sensor[n];
    if (n==0 && a->literal[0]) {
      byte t[4];
      memcpy(t,a->literal,4);
      if (a->alertState[n])
        t[3] = (a->alertState[1] & (WARNING_LOW | CAUTION_LOW)) ? LED_L : LED_H;
      printLED(n,t);
    }
    else if (s->pin & DUAL_BIT)
      printLEDFuel(scaleValue(s, readSensor(s,0)), scaleValue(s, readSensor(s,1)));    // Show the dual fuel gauge
    else if (s) {
	  short v = scaleValue(s, readSensor(s));
      printLED(n,v, s->decimal);
	}
  }
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
  if (blinkAux[0] > 0) {
    blinkAux[0] = -1;
    ack = true;
  }
  if (blinkAux[1] > 0) {
    blinkAux[1] = -1;
    ack = true;
  }
  return ack;
}



// Press (acknowedge/next-page)
inline void buttonPress() {
  if (ackBlink()) {
    ackAlert();
  }
  else {
    if (ackAlert())
      auxPage = AUX_STARTUP_PAGES;
    else {
      auxPage++;
      if (auxPage >= N(auxDisplay))
        auxPage = AUX_STARTUP_PAGES;
    }
  }
}

// Hold (reshow all alerts)
inline void buttonHold() {
  blinkAux[0] = blinkAux[1] = 0;
  for (byte i=AUX_STARTUP_PAGES; i<N(auxDisplay); i++)
    auxDisplay[i].warning = auxDisplay[i].caution = 0;
  auxPage = AUX_STARTUP_PAGES;
  didHoldKey = true;
}

// Long hold (toggle dim and bright)
inline void buttonLongHold() {
  dimAux = !dimAux;
  for (byte line=0; line<2; line++)
    commandLED(line, dimAux?HT16K33_BRIGHT_MIN:HT16K33_BRIGHT_MAX);
  didChangeDim = true;
  didHoldKey = false;
}

inline void checkAuxSwitch() {
  if (switchPress > 0) {
    if (switchPress < 8)
      buttonPress();
    switchPress = 0;
    didChangeDim = false;
    didHoldKey = false;
  }

  if (switchDown >= 24 && !didChangeDim)
    buttonLongHold();
  else if (switchDown >= 8 && !didHoldKey)
    buttonHold();

  if (switchUp > SHOW_DEFAULT_AUX_PAGE_TIMEOUT*8) {
    if (auxDisplay[auxPage].warning <=0 && auxDisplay[auxPage].caution <= 0)
      auxPage = AUX_STARTUP_PAGES;
  }
}

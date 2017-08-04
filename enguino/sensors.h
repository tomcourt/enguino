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



// Digital pin assignments
#define FUELF_SIGNAL 1 
#define TACH_SIGNAL  0

#define RVoff           -495                    // 495 ADC units is when ADC when gauge reads 0, using a 240-33 ohm sensor and a 240 ohm divider
#define RVscale         (1000.0/(124+RVoff))    // 124 ADC units is when ADC when gauge reads max, using same

#define HOBBS_COUNT_INTERVAL (3600/40)    // update hobbs 40 times an hour

extern volatile short tcTemp[9];    // in quarter deg. C, tcTemp[8] is the interal reference temp, disable IRQ's to access these

short adcSample[12][4];
byte adcIndex;

volatile bool tachDidPulse;
volatile short rpm[8];

volatile word fflowCount;
volatile word fflowRunning;
word fflowRate;     // in k-factor counts for last 2 seconds, ~3.8 counts is .1 GPH
word ffRunning[4];
byte ffRunningInx;

byte hobbsCount = HOBBS_COUNT_INTERVAL/2;  // in order to prevent cumulative hobbs error assume half a hobbs count of engine run time was lost on last shutdown


static const byte thermistorBS[] = { 
  3,3,3,3,3,3,4,4,
  4,4,4,4,4,5,5,5,
  5,5,6,6,7,6,6,5,
  5,5,5,4,4,4,4    
};

static const short thermistorYV[] = { 
  1500,1438,1384,1336,1293,1254,1219,1155,
  1100,1051,1008,968,932,898,838,784,
  736,692,650,575,505,375,310,241,
  204,164,121,72,44,14,-19,-58
};

const InterpolateTable thermistor = { 64, 32, thermistorBS, thermistorYV }; 

static const byte r240to33BS[] = { 
  5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 
};

static const short r240to33YV[] = { 
  1105, 1064, 1019, 972, 921, 894, 866,
   837,  806,  775, 742, 707, 671, 634,
   595,  553,  510, 465, 417, 367, 314,
   258,  199,  137,  70,   0, -75, -155  
};

const InterpolateTable r240to33 = { 48,  28, r240to33BS, r240to33YV };



#if SIMULATE_SENSORS
#define ADC_MP(x) (short)(1000L*((x)-3.117)/32.811 +.5)

byte simState = 0;

short simulate[3][24] = {
{  
  1000L*14/20,          // 14 v
  // 1024*12/20,          // 12 v
  1000L*60/100,    // OP - 60 psi
  200*10,       // 200 deg-F
  1000L*4/15,   // FP - 4 psi           
  1000L*10/16,    // 10 gal
  1000L*2/16,   // 2 gal
  0, 0,
  0, ADC_MP(30), 0, 0, 4321, 3500, 121, 2800,
  310*4,320*4,330*4,340*4,        // CHT
  1100*4,1200*4,1300*4,1400*4     // EGT
},
{  
  1000L*12/20,          // 12 v
  1000L*60/100,    // OP - 60 psi
  200*10,       // 200 deg-F
  1000L*4/15,   // FP - 4 psi
  1000L*10/16,    // 10 gal
  1000L*5/16,   // 5 gal
  0, 0,
  0, ADC_MP(20), 0, 0, 4321, 3500, 122, 2300,
  310*4,320*4,330*4,340*4,        // CHT
  1100*4,1200*4,1300*4,1400*4     // EGT
},
{  
  1000L*12/20,          // 14 v
  1000L*2/100,    // OP - 60 psi
  200*10,       // 200 deg-F
  1000L*4/15,   // FP - 4 psi
  1000L*10/16,    // 10 gal
  1000L*5/16,   // 5 gal
  0, 0,
  0, ADC_MP(10), 0, 0, 4321, 3500, 123, 0, 
  310*4,320*4,330*4,340*4,        // CHT
  1100*4,1200*4,1300*4,1400*4     // EGT
}
};
#endif

// Call this every half second (in an IRQ)
inline void updateFuelFlow() {
  ffRunningInx++;
  ffRunningInx &= 3;
  fflowRate = fflowRunning - ffRunning[ffRunningInx];
  ffRunning[ffRunningInx] = fflowRunning;
}

inline void updateTach() {
  noInterrupts();
  if (tachDidPulse)
    tachDidPulse = false;
  else
    memset((void *)rpm, 0, sizeof(rpm));
  interrupts();
}

inline void updateHobbs() {
  if (--hobbsCount == 0) {
    if (++(ee_status.hobbs) > 39999) {
      ee_status.hobbs = 0;
      ee_status.hobbs1k++;
    }
    eeUpdateDirty = true;
    hobbsCount = HOBBS_COUNT_INTERVAL;
  }
}

void tachIRQ() {
  static byte rpmInx;
  static unsigned long lastTachTime;

  unsigned long newTachTime = micros();
  rpm[rpmInx++ & 7] = (60000000L/TACH_DIVIDER) / (newTachTime - lastTachTime);
  lastTachTime = newTachTime;
  tachDidPulse = true;
}

void fflowIRQ() {
  fflowCount++;
  fflowRunning++;
  if (fflowCount >= ee_settings.kFactor) {
    if (ee_status.fuel > 0) {
      ee_status.fuel--;
      eeUpdateDirty = true;
    }
    fflowCount = 0;
  }
}

// takes about 1.2 mS
void updateADC() {
  for (byte i=0; i<12; i++)
    adcSample[i][adcIndex] = analogRead(i);
  adcIndex++;
  adcIndex &= 3;
}

short average4(short *samples) {
  short avg = 0;
  for (byte i=0; i<4; i++)
    avg += *samples++;
  return avg>>2;
}

void sensorSetup() {
 // RPM's are measured as microseconds between IRQ's. Occasional latency means 'wild' readings need to be thrown away
  attachInterrupt(digitalPinToInterrupt(TACH_SIGNAL),tachIRQ,RISING);

  // Fuel totalizer is assumed to be a 'Red Cube'
  pinMode(FUELF_SIGNAL, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FUELF_SIGNAL),fflowIRQ,RISING);  

  fflowCount = ee_settings.kFactor >> 1;  // in order to prevent cumulative fuel consumed error assume half a k-factor of a gallon was lost on last shutdown
}

short readSensor(const Sensor *s, byte n = 0) {
  short p = s->pin + n;
  
#if SIMULATE_SENSORS
    if (p < 0)
      return FAULT;
    return simulate[simState][p&(DUAL_BIT-1)]; 
#else
    short v;
    if (p < 0)
      return FAULT;
    p &= (DUAL_BIT-1);
    
    short t = s->type;
    short toF = 0;
    if (p == HOBBS_SENSOR) { // Hobbs hours*10 (lowest 4 digits)
      v = ee_status.hobbs >> 2;
    }
    else if (p == FUELF_SENSOR) {      // Fuel flow GPH*10
      noInterrupts();
      v = short((multiply(fflowRate, int(10*3600L/40/2)) + (ee_settings.kFactor>>1)) / ee_settings.kFactor);
      interrupts();
    }
    else if (p == FUELR_SENSOR) { // Fuel remaining Gallons*10 (totalizer)
      noInterrupts();
      v = ee_status.fuel >> 2;
      interrupts();
    }
    else if (p == TACH_SENSOR) {  // RPM's
      // RPM's are occasionaly screwed up because of IRQ latency.
      // Throw out highest and lowest and average the middle
      noInterrupts();
      short r[N(rpm)];
      memcpy(r, rpm, sizeof(rpm));
      interrupts();
      sort(r, N(r));
      v = average4(r + N(r)/2 - 2);    // average the middlemost 4 
    }
    else if (p < 16) { 
      v = average4(adcSample[p]);
      if (t == st_r240to33)
        v = interpolate(&r240to33, v);
      else if (t == st_v240to33)
        v = multiplyAndScale(SCALE(RVscale),v+RVoff, divisor);
      else if (t == st_thermistorC || t == st_thermistorF) {
        v = interpolate(&thermistor, v);
        if (t == st_thermistorF)
          toF = 32 * 10;
      }
      else if (t == st_volts)
        v = multiplyAndScale(v,1000,10);
    }
    else if (p < 24) {     
      noInterrupts();
      v = tcTemp[p-16];
      interrupts();
      if (t == st_j_type_tcC || t == st_j_type_tcF)
       v = multiplyAndScale(v - tcTemp[8], 25599, 15) + tcTemp[8]; 
      if (t == st_k_type_tcF || t == st_j_type_tcF)
        toF = 32 * 4;
    }
    if (toF && v != FAULT) 
      v = (v*9)/5 + toF;
    return v;     
#endif
}

short scaleValue(const Sensor *s, short val) {
  if (val == FAULT)
    return FAULT;
  return multiplyAndScale(s->vfactor,val+s->voffset, divisor);
}

byte alertState(const Sensor *s, byte offset) {
 byte b = 0;
  if (s) {
    short v = scaleValue(s, readSensor(s,offset));
    if (v == FAULT) 
      b = ALERT_FAULT;
    else {
      if (v < s->lowWarning)
        b |= WARNING_LOW;
      else if (v < s->lowCaution)
        b |= CAUTION_LOW;
      if (v > s->highWarning)
        b |= WARNING_HIGH;
      else if (v > s->highCaution)
        b |= CAUTION_HIGH;
    }
  }
  return b;
}

inline bool isEngineRunning() {
  return !(bool(RUN_VOLT) || bool(RUN_OILP) || bool(RUN_TACH))
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
}





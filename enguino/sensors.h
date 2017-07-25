#define RVoff           -495                    // 495 ADC units is when ADC when gauge reads 0, using a 240-33 ohm sensor and a 240 ohm divider
#define RVscale         (1000.0/(124+RVoff))    // 124 ADC units is when ADC when gauge reads max, using same

extern volatile int tcTemp[9];    // in quarter deg. C, tcTemp[8] is the interal reference temp, disable IRQ's to access these

int adcSample[12][4];
byte adcIndex;

const InterpolateTable thermistor = {
  64, 32,
  (byte []) { 
    3,3,3,3,3,3,4,4,
    4,4,4,4,4,5,5,5,
    5,5,6,6,7,6,6,5,
    5,5,5,4,4,4,4    
  },
  (int []) { 
    1500,1438,1384,1336,1293,1254,1219,1155,
    1100,1051,1008,968,932,898,838,784,
    736,692,650,575,505,375,310,241,
    204,164,121,72,44,14,-19,-58
  }
};

const InterpolateTable r240to33 = {
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
  }
};

#if SIMULATE_SENSORS
byte simState = 1;

int simulate1[24] = {  
  1000L*14/20,          // 14 v
  // 1024*12/20,          // 12 v
  1000L*60/100,    // OP - 60 psi
  200*10,       // 200 deg-F
  1000L*4/15,   // FP - 4 psi           
  1000L*10/16,    // 10 gal
  1000L*2/16,   // 2 gal
  0, 0,
  0, 0, 0, 0, 0, 0, 0, 2800,
  310*4,320*4,330*4,340*4,      // CHT
  1010*4,1020*4,1030*4,1040*4     // EGT
};

int simulate2[24] = {  
  1000L*12/20,          // 12 v
  1000L*60/100,    // OP - 60 psi
  200*10,       // 200 deg-F
  1000L*4/15,   // FP - 4 psi
  1000L*10/16,    // 10 gal
  1000L*5/16,   // 5 gal
  0, 0,
  0, 0, 0, 0, 0, 0, 0, 2300,
  310*4,320*4,330*4,340*4,      // CHT
  1010*4,1020*4,1030*4,1040*4     // EGT
};

int simulate3[24] = {  
  1000L*14/20,          // 14 v
  1000L*60/100,    // OP - 60 psi
  200*10,       // 200 deg-F
  1000L*4/15,   // FP - 4 psi
  1000L*10/16,    // 10 gal
  1000L*5/16,   // 5 gal
  0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  310*4,320*4,330*4,340*4,      // CHT
  1010*4,1020*4,1030*4,1040*4     // EGT
};

#endif

// takes about 1.2 mS
void updateADC() {
  for (byte i=0; i<12; i++)
    adcSample[i][adcIndex] = analogRead(i);
  adcIndex++;
  adcIndex &= 3;
}

int average4(int *samples) {
  int avg = 0;
  for (byte i=0; i<4; i++)
    avg += *samples++;
  return avg>>2;
}


int readSensor(const Sensor *s, byte n = 0) {
  int p = s->pin + n;
  
  #if SIMULATE_SENSORS
    if (p < 0)
      return FAULT;
    p &= (DUAL_BIT-1);
    switch(simState) {
      case 1: return simulate1[p];  
      case 2: return simulate2[p];  
      case 3: return simulate3[p];  
    }    
  #else
    int v;
    if (p < 0)
      return FAULT;
    p &= (DUAL_BIT-1);
    
    int t = s->type;
    int toF = 0;
    if (p == 15) {
      // RPM's are occasionaly screwed up because of IRQ latency.
      // Throw out highest and lowest and average the middle
      noInterrupts();
      int r[N(rpm)];
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

int scaleValue(const Sensor *s, int val) {
  if (val == FAULT)
    return FAULT;
  return multiplyAndScale(s->vfactor,val+s->voffset, divisor);
}

byte alertState(Sensor *s, byte offset) {
 byte b = 0;
  if (s) {
    int v = scaleValue(s, readSensor(s,offset));
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




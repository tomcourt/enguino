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
#define R2ADC(r) (int)((1024.0 * (r) / ((r)+240.0)) + 0.5)

byte simState = 1;

int simulate1[24] = {  
  1024*14/20,          // 14 v
  // 1024*12/20,          // 12 v
  R2ADC(72),    // OP - 60 psi
  200*10,       // 200 deg-F
  R2ADC(123),   // FP - 4 psi           
  // R2ADC(220),   // FP - .4 psi
  R2ADC(86),    // 10 gal
  R2ADC(200),   // 2 gal
  0, 0,
  0, 0, 0, 0, 0, 0, 0, 2600,
  310*4,320*4,330*4,340*4,      // CHT
  1010*4,1020*4,1030*4,1040*4     // EGT
};

int simulate2[24] = {  
  1024*12/20,          // 12 v
  R2ADC(72),    // OP - 60 psi
  200*10,       // 200 deg-F
  R2ADC(123),   // FP - 4 psi
  R2ADC(86),    // 10 gal
  R2ADC(138),   // 5 gal
  0, 0,
  0, 0, 0, 0, 0, 0, 0, 2300,
  310*4,320*4,330*4,340*4,      // CHT
  1010*4,1020*4,1030*4,1040*4     // EGT
};

int simulate3[24] = {  
  1024*14/20,          // 14 v
  R2ADC(72),    // OP - 60 psi
  200*10,       // 200 deg-F
  R2ADC(123),   // FP - 4 psi
  R2ADC(86),    // 10 gal
  R2ADC(138),   // 5 gal
  0, 0,
  0, 0, 0, 0, 0, 0, 0, 2300,
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
    switch(simState) {
      case 1: return simulate1[p];  
      case 2: return simulate2[p];  
      case 3: return simulate3[p];  
    }
//    if (p < 16)
//      return rand() & 0x3ff;        
//    if (p < 20)                       
//      return rand() & 0x7ff;          // CHT
//    if (p < 24)
//      return rand() & 0x7ff + 4000;   // EGT 
//    return FAULT;
    
  #else
    int v;
    if (p < 0)
      return FAULT;
     
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
       v = multiplyAndScale(v - tcTemp[8], 25599, 15) + tcTemp[8]; 
      if (t == st_k_type_tcF || t == st_j_type_tcF)
        toF = 32 * 4;
    }
    if (toF && v != FAULT) 
      v = (v*9)/5 + toF;
    return v;     
  #endif
}


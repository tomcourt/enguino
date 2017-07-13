extern volatile int tcTemp[9];    // in quarter deg. C, tcTemp[8] is the interal reference temp, disable IRQ's to access these


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
      sort(r, N(r));
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
       v = multiplyAndScale(v - tcTemp[8], 25599, 15) + tcTemp[8]; 
      if (t == st_k_type_tcF || t == st_j_type_tcF)
        toF = 32 * 4;
    }
    if (toF && v != FAULT) 
      v = (v*9)/5 + toF;
    return v;     
  #endif
}


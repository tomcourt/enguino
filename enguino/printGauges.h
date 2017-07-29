/// Implementation for printPrefix and pringGauge

int scaleMark(const Sensor *s, int val) {
  if (val == FAULT)
    return FAULT;
  int mark = multiplyAndScale(s->gfactor, val+s->goffset, divisor);
  if (mark < 0)
    mark = 0;
  if (mark > 1000)
    mark = 1000;
  return mark;
}

void printGaugeRawValue(int x, int y, Sensor *s, int val, byte pinOffset) {
  print_g_translate(x, y);
  byte alert = alertState(s, pinOffset);
  if (alert) {
    print_P(F("'<rect width='1000' height='500' rx='90' ry='90' fill='"));
    if (alert & WARNING_ANY)
      print(red);
    else
      print(yellow);
    print_n_close();
  }
  print_P(F("<text x='500' y='425' class='value'>"));
  print(val, s->decimal);
  print_text_close();  
  print_g_close();
}

void printGaugeValue(int x, int y, Sensor *s, byte pinOffset=0) {
  printGaugeRawValue(x, y, s, scaleValue(s, readSensor(s, pinOffset)), pinOffset);
}

// vertical gauge is
//    1200 wide except extra room on right needed for labels (centered at 600)
//    5950 high
void printVertical(const Gauge *g, bool showLabels=true, byte pinOffset=0) {
#ifdef BOUNDING_BOX
  print_P(F("<rect x='0' y='0' width='1200' height='5950' fill='none' stroke='orange'/>\n"));
#endif
  // starts at 1100, 4000 high
  print_P(F("<rect x='400' y='1000' width='400' height='4000' class='rectgauge'/>\n"));
  
   // fill in the color regions of the gauage
  int s = 0; // start at bottom and work up
  for (int i=0; i<g->n_regions; i++) {
    int e = g->regionEndPts[i];
    print_P(F("<rect fill='"));
    print(g->regionColors[i]);
    print_P(F("' x='400' y='"));
    print(4000 + 1000 - e);
    print_P(F("' width='400' height='"));
    print_n_close(e-s);
    s = e;
  }
  
  // add tick marks and labels
  for (int i=0; i<g->n_labels; i++) {
    s = 4000 + 1000 - g->labelPts[i];
    print_P(F("<line class='segment' x1='250' y1='"));
    print(s);
    print_P(F("' x2='950' y2='"));
    print_n_close(s);
    if (showLabels) {
      print_P(F("<text x='1000' y='"));
      print(s);
      print_P(F("' class='number'>"));
      print(g->labelValues[i]);
      print_text_close();
    }
  }
  
  if (showLabels) {
    print_P(F("<text x='600' y='400' class='label'>"));
    print_text_close(g->label1);
  }
    
  print_P(F("<text x='600' y='820' class='label'>"));
  print_text_close(g->label2);
  
  print_P(F("<text x='600' y='5800' class='unit'>"));
  print_text_close(g->units);

  printGaugeValue(100, 5075, g->sensor, pinOffset);
 
  int mark = scaleMark(g->sensor, readSensor(g->sensor, pinOffset)) << 2;
  if (mark != FAULT) {
    print_P(F("<use xlink:href='#vmark' x='600' y='"));
    print_n_close(4000 + 1000 - mark);
  }
}


// horizontal gauge is
//    ... wide except (centered at ...)
//    ... or so high
void printHorizontal(const Gauge *g, int count) {
  // starts at ..., 8000 wide
#ifdef BOUNDING_BOX
  print_P(F("<rect x='-2' y='0' width='9100' height='3500' fill='none' stroke='orange'/>\n"));
#endif

  int offset = 0;
  for (int n=0; n<count; n++) {
    print_P(F("<rect x='1100' y='"));
    print(600+offset);
    print_P(F("' width='8000' height='400' class='rectgauge'/>\n"));
        
    // fill in the color regions of the gauage
    int s = 0; // start at left and work right
    for (int i=0; i<g->n_regions; i++) {
      int e = g->regionEndPts[i];
      print_P(F("<rect fill='"));
      print(g->regionColors[i]);
      print_P(F("' x='"));
      print(1100 + s);
      print_P(F("' y='"));
      print(600 + offset);
      print_P(F("' height='400' width='"));
      print_n_close(e-s);     
      s = e;
    }
    
    printGaugeValue(0, offset+550, g->sensor, n);

    int mark = scaleMark(g->sensor, readSensor(g->sensor, n)) << 3;
    if (mark != FAULT) {
      print_P(F("<use xlink:href='#hmark' y='"));
      print(offset+800);
      print_P(F("' x='"));
      print_n_close(1100 + mark);
    }
    offset += 800;
  }
  
  // add tick marks and labels
  for (int i=0; i<g->n_labels; i++) {
    int s = g->labelPts[i] + 1100;
    
    print_P(F("<line class='segment' y1='450' x1='"));
    print(s);
    print_P(F("' y2='"));
    print(offset+350);
    print_P(F("' x2='"));
    print_n_close(s);
    
    print_P(F("<text y='300' x='"));
    print(s);
    print_P(F("' class='mnumber'>"));
    print(g->labelValues[i]);
    print_text_close();
  }
  
  print_P(F("<text x='500' y='250' class='label' alignment-baseline='central';>"));
  print_text_close(g->label1);
}


void printEGTHoriz(const Gauge *g, int count) {
  // starts at ..., 8000 wide
#ifdef BOUNDING_BOX
  print_P(F("<rect x='1100' y='0' width='9300' height='3850' fill='none' stroke='orange'/>\n"));
#endif
  int offset = 0;
  for (int n=0; n<count; n++) {
    int val = scaleValue(g->sensor, readSensor(g->sensor, n));
    if (leanMode && val != FAULT) {
      if (val > peakEGT[n])
        peakEGT[n] = val;
      if (val+5 < peakEGT[n])  // minimium of 5 deg. F drop before showing negative
        val -= peakEGT[n];
    }

    printGaugeRawValue(9200,offset+550,g->sensor,val,n);
    
    int mark = scaleMark(g->sensor, readSensor(g->sensor, n)) << 3;
    if (mark != FAULT) {
      print_P(F("<use xlink:href='#xmark' y='"));
      print(offset + 800);
      print_P(F("' x='"));
      print_n_close(1100 + mark);
    }
    offset += 800;
  }
  
  // add labels
  for (int i=0; i<g->n_labels; i++) {
    print_P(F("<text y='"));
    print(offset + 500);
    print_P(F("' x='"));
    print(g->labelPts[i] + 1100);
    print_P(F("' class='mnumber'>"));
    print(g->labelValues[i]);
    print_text_close();
  }
  
  print_P(F("<text x='9700' y='250' class='label' alignment-baseline='central';>"));
  print_text_close(g->label1);
}



// vertical pair of gauges is
//    2700 wide (centered at 1350)
//    5950 or so high
void printVerticalPair(const Gauge *g) {
#ifdef BOUNDING_BOX
  print_P(F("<rect x='0' y='0' width='2700' height='5950' fill='none' stroke='orange'/>\n"));
#endif

  print_P(F("<text x='1350' y='400' class='label'>"));
  print_text_close(g->label1);
  
  Gauge tg = *g;
  tg.label2 = "LEFT";
  printVertical(&tg, false, 0);
  tg.label2 = "RGT";
  print_g_translate(1500,0);
  printVertical(&tg, false, 1);
  print_g_close();
  // add tick marks and labels
  for (int i=0; i<g->n_labels; i++) {
    print_P(F("<text x='1350' y='"));
    print(1000 + 4000 - g->labelPts[i]);
    print_P(F("' class='mnumber'>"));
    print(g->labelValues[i]);
    print_text_close();
  }
}

// round gauge sweeps is
//    3000 wide (centered at 1500)
//    2650 or so high
void printRound(const Gauge *g) {
#ifdef BOUNDING_BOX
  print_P(F("<rect x='0' y='0' width='3000' height='2650' fill='none' stroke='orange'/>\n"));
#endif

  print_g_translate(1500,1800);   // center of the dial is origin
  // border sweeps from -31 to 31 degrees, use code below to figure out magic x,y values in path
  //    logValue(ARCX(-.004),"x -");
  //    logValue(ARCY(-.004),"y -");

  // all dimensions are multiplied by 10 for arc drawing because even small integer 
  // truncation errors in sub-arcs cause them to not align well with the black backdrop arc.
  print_P(F("<g transform='scale(.1)'>\n"));    
  print_P(F("<path d='M-11147 6688 A 13000 13000 0 1 1 11147 6688' fill='none' stroke='black' stroke-width='4500' />\n"));
  
  // fill in the color regions of the gauage
  int x0 = ARCX(0), y0 = ARCY(0); // far left of sweep
  for (int i=0; i<g->n_regions; i++) {
    int x1 = g->regionEndPts[i];
    int y1 = g->regionEndPts[i + g->n_regions];
    
    print_P(F("<path d='M"));
    print(x0);
    print(' ');
    print(y0);
    print_P(F("A 13000 13000 0 "));
    print(g->regionEndPts[i + g->n_regions*2]);
    print_P(F(" 1 "));
    print(x1);
    print(' ');
    print(y1);
    print_P(F("' fill='none' stroke-width='4000' stroke='"));
    print(g->regionColors[i]);
    print_n_close();
    
    x0 = x1, y0 = y1;
  }
  print_g_close();
  
  print_P(F("<text x='0' y='-200' class='label'>"));
  print_text_close(g->label1);
  
  print_P(F("<text x='0' y='5900' 700='unit'>"));
  print_text_close(g->units);
  
  printGaugeValue(-500, -80, g->sensor);
 
  print_P(F("<text x='0' y='700' class='unit'>"));
  print_text_close(g->units);

  int mark = scaleMark(g->sensor, readSensor(g->sensor));
  if (mark != FAULT) {
    print_P(F("<use xlink:href='#vmark' x='-1300' y='0' transform='rotate("));
    print((mark*24) / 10 - 300, 1);   // convert 0 to 1000 scale value to -30.0 to 210.0 degrees of angle
    print_P(F(")'/>\n"));
  }
  print_g_close();
}


void printInfoBox() {
#ifdef BOUNDING_BOX
  print_P(F("<rect x='0' y='0' width='1600' height='2800' fill='none' stroke='orange'/>\n"));
#endif

  print_P(F("<g width='1600' height='600' onClick=\"javascript:"));
  if (engineRunning)
    print_P(F("ajax('?x=l');\">\n"));
  else
    print_P(F("location.assign('s');\">\n"));  
  print_P(F("<rect width='1600' height='600' rx='100' ry='100' class='abutton'/>\n"
  "<text x='800' y='475' class='value'>"));
  if (!engineRunning)
    print_P(F("Setup"));
  else if (leanMode)
    print_P(F("Cancel"));
  else
    print_P(F("Lean"));
  print_P(F("</text></g>\n"));
  
  print_P(F("<text x='800' y='1300' class='value'>GPH: "));    
  print(readSensor(&fuelfS), 1);
  print_text_close();
    
  print_P(F("<text x='800' y='2100' class='value'>Tot: "));    
  print(readSensor(&fuelrS), 1);
  print_text_close();
 
  print_P(F("<text x='800' y='2800' style='text-anchor:middle; font-size:300px;'>Hobb: "));    
  if (ee_status.hobbs1k)
    print(ee_status.hobbs1k);
  print(ee_status.hobbs >> 2, 1);
  print_text_close();
}


void printGauge(const Gauge *g) {
  print_g_translate(g->x, g->y);
  switch(g->style) {
    case gs_vert:
      printVertical(g);
      break;
    case gs_pair:
      printVerticalPair(g);
      break;
    case gs_round:
      printRound(g);
      break;
    case gs_horiz:
      printHorizontal(g, 4);
      break;
    case gs_egt:
      printEGTHoriz(g, 4);
      break;
    case gs_infobox:
      printInfoBox();
      break;
  }
  print_g_close();
}




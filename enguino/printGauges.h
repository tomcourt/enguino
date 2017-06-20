/// Implementation for printPrefix and pringGauge

int scaleMark(const Sensor *s, int val) {
  int mark = (int)(((long)s->mfactor * (long)val) >> divisor) + s->moffset;
  if (mark < 0)
    mark = 0;
  if (mark > 1000)
    mark = 1000;
  return mark;
}

int scaleValue(const Sensor *s, int val) {
  if (val == SHORT || val == OPEN)
    return val;
  return (int)(((long)s->vfactor * (long)val) >> divisor) + s->voffset;
}

// vertical gauge is
//    1200 wide except extra room on right needed for labels (centered at 600)
//    6050 or so high
void printVertical(const Gauge *g, bool showLabels) {
  // starts at 1100, 4000 high
  print_P(F("<rect x='400' y='1100' width='400' height='4000' class='rectgauge'/>\n"));
  
  int val = readPin(g->pin);
  int mark = scaleMark(g->sensor, val) << 2;
  
  const char *color = 0;
  
  // fill in the color regions of the gauage
  int s = 0; // start at bottom and work up
  for (int i=0; i<g->n_regions; i++) {
    int e = g->regionEndPts[i];
    if (mark >= s && mark <= e)
      color = g->regionColors[i];
    print_P(F("<rect fill='"));
    print(g->regionColors[i]);
    print_P(F("' x='400' y='"));
    print(4000 + 1100 - e);
    print_P(F("' width='400' height='"));
    print_n_close(e-s);
    s = e;
  }
  
  // add tick marks and labels
  for (int i=0; i<g->n_labels; i++) {
    s = g->labelPts[i] + 1100;
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
    print_P(F("<text x='600' y='500' class='label'>"));
    print_text_close(g->label1);
  }
    
  print_P(F("<text x='600' y='920' class='label'>"));
  print_text_close(g->label2);
  
  print_P(F("<text x='600' y='5900' class='unit'>"));
  print_text_close(g->units);
  
  if (color != 0 && color != green) {
    print_P(F("<rect x='100' y='5175' width='1000' height='500' rx='90' ry='90' fill='"));
    print(color);
    print_n_close();
  }

  print_P(F("<text x='600' y='5600' class='value'>"));
  print(scaleValue(g->sensor, val), g->decimal);
  print_text_close();
  
  print_P(F("<use xlink:href='#vmark' x='600' y='"));
  print_n_close(4000 + 1100 - mark);
}


// horizontal gauge is
//    ... wide except (centered at ...)
//    ... or so high
void printHorizontal(const Gauge *g, int count) {
  // starts at ..., 8000 wide
  int offset = 0;
  for (int n=0; n<count; n++) {
    print_P(F("<rect x='1100' y='"));
    print(600+offset);
    print_P(F("' width='8000' height='400' class='rectgauge'/>\n"));
    
    int val = readPin(g->pin + n);
    int mark = scaleMark(g->sensor, val) << 3;
    
    const char *color = 0;
    
    // fill in the color regions of the gauage
    int s = 0; // start at left and work right
    for (int i=0; i<g->n_regions; i++) {
      int e = g->regionEndPts[i];
      if (mark >= s && mark <= e)
        color = g->regionColors[i];
      
      print_P(F("<rect fill='"));
      print(g->regionColors[i]);
      print_P(F("' x='"));
      print(1100 + s);
      print_P(F("' y='"));
      print(600+offset);
      print_P(F("' height='400' width='"));
      print_n_close(e-s);
      
      s = e;
    }
    
    if (color != 0 && color != green) {
      print_P(F("<rect x='0' y='"));
      print(offset+550);
      print_P(F("' width='1000' height='500' rx='90' ry='90' fill='"));
      print(color);
      print_n_close();
    }

    print_P(F("<text x='500' y='"));
    print(offset+800);
    print_P(F("' class='value' alignment-baseline='central'>"));
    print(scaleValue(g->sensor, val), g->decimal);
    print_text_close();
    
    print_P(F("<use xlink:href='#hmark' y='"));
    print(offset+800);
    print_P(F("' x='"));
    print_n_close(1100 + mark);
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


void printAuxHoriz(const Gauge *g, int count) {
  // starts at ..., 8000 wide
  int offset = 0;
  for (int n=0; n<count; n++) {
    int val = readPin(g->pin + n);
    int mark = scaleMark(g->sensor, val) << 3;
    
    print_P(F("<text x='9700' y='"));
    print(offset + 800);
    print_P(F("' class='value' alignment-baseline='central'>"));
    print(scaleValue(g->sensor, val), g->decimal);
    print_text_close();
    
    print_P(F("<use xlink:href='#xmark' y='"));
    print(offset + 800);
    print_P(F("' x='"));
    print_n_close(1100 + mark);
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
//    6050 or so high
void printVerticalPair(const Gauge *g) {
  print_P(F("<text x='1350' y='500' class='label'>"));
  print_text_close(g->label1);
  
  Gauge t = *g;
  t.label2 = "LEFT";
  printVertical(&t, false);
  t.label2 = "RGT";
  t.pin++;
  print_P(F("<g transform='translate(1500 0)'>"));
  printVertical(&t, false);
  print_g_close();
  // add tick marks and labels
  for (int i=0; i<g->n_labels; i++) {
    print_P(F("<text x='1350' y='"));
    print(g->labelPts[i] + 1100);
    print_P(F("' class='mnumber'>"));
    print(g->labelValues[i]);
    print_text_close();
  }
}

// round gauge is
//    3000 wide (centered at 1500)
//    2650 or so high
void printRound(const Gauge *g) {
  // gauge sweeps 2400 units (-30.0 to 30.0 degrees)
  print_P(F("<g transform='translate(1500 1800)'>\n"));
  // border sweeps from -31 to 31 degrees
  print_P(F("<path d='M-1114 670 A 1300 1300 0 1 1 1114 670' fill='none' stroke='black' stroke-width='450' />\n"));
  
  // fill in the color regions of the gauage
  int x0 = -1126, y0 = 650; // far left of sweep
  for (int i=0; i<g->n_regions; i++) {
    int x1 = g->regionEndPts[i];
    int y1 = g->regionEndPts[i + g->n_regions];
    
    print_P(F("<path d='M"));
    print(x0);
    print(' ');
    print(y0);
    print_P(F("A 1300 1300 0 "));
    print(g->regionEndPts[i + g->n_regions*2]);
    print_P(F(" 1 "));
    print(x1);
    print(' ');
    print(y1);
    print_P(F("' fill='none' stroke-width='400' stroke='"));
    print(g->regionColors[i]);
    print_n_close();
    
    x0 = x1, y0 = y1;
  }
  
  print_P(F("<text x='0' y='-200' class='label'>"));
  print_text_close(g->label1);
  
  print_P(F("<text x='0' y='5900' 700='unit'>"));
  print_text_close(g->units);
  
  int val = readPin(g->pin);
  int mark = (scaleMark(g->sensor, val) * 24) / 10;

  int scale = scaleValue(g->sensor, val);

  // hard coded for tachometer
  if (g->pin == 6) {
    const char *color = 0;
    if (scale < 500)
      color = yellow;
    if (scale > 2700)
      color = red;
  
    if (color) {
      print_P(F("<rect x='-500' y='-80' width='1000' height='500' rx='90' ry='90' fill='"));
      print(color);
      print_n_close();
    }
  }

  print_P(F("<text x='0' y='350' class='value'>"));
  print(scale, g->decimal);
  print_text_close();
  
  print_P(F("<text x='0' y='700' class='unit'>"));
  print_text_close(g->units);
  
  print_P(F("<use xlink:href='#vmark' x='-1300' y='0' transform='rotate("));
  print(mark-300,1);
  print_P(F(")'/>\n"));
  print_g_close();
}


void printGauge(const Gauge *g) {
  print_P(F("<g transform='translate("));
  print(g->x);
  print(' ');
  print(g->y);
  print_P(F(")'>\n"));
  switch(g->style) {
    case gs_vert:
      printVertical(g, true);
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
    case gs_aux:
      printAuxHoriz(g, 4);
      break;
  }
  print_g_close();
}


void printPrefix() {
  print_P(F(
  "<!DOCTYPE html>\n"
  "<html>\n"
    "<head>\n"
      "<title>Enguino</title>\n"
      "<meta name='apple-mobile-web-app-capable' content='yes'>\n"      
      "<meta name='mobile-web-app-capable' content='yes'>\n"      
      "<style>\n"
        ".segment { stroke:gray; stroke-width:20; }\n"
        ".rectgauge  {  fill:none; stroke:black; stroke-width:40; }\n"
        ".roundgauge {  fill:none; stroke:black; }\n"
        ".label  { fill:dimgrey; text-anchor:middle; font-size:500px; }\n"
        ".value  { fill:black; text-anchor:middle; font-size:500px; }\n"
        ".number { fill:dimgrey; text-anchor:start; font-size:300px; alignment-baseline:central; }\n"
        ".mnumber { fill:dimgrey; text-anchor:middle; font-size:300px; alignment-baseline:central; }\n"
        ".unit   { fill:dimgrey; text-anchor:middle; font-size:300px; }\n"
        ".abutton { fill:lightgrey; stroke:black; stroke-width:40; }\n"
        ".indicator { fill:black }\n"
      "</style>\n"
      "<script type='text/javascript'>\n"
        "setInterval(function() {\n"
          "var xhttp = new XMLHttpRequest();\n"
          "xhttp.onreadystatechange = function() {\n"
            "if (this.readyState == 4 && this.status == 200) {\n"
              "document.getElementById('dyn').innerHTML =\n"
              "this.responseText;\n"
            "}\n"
          "};\n"
          "xhttp.open('GET', 'd', true);\n"
          "xhttp.send();\n"
        "}, 1000);\n"
      "</script>\n"
    "</head>\n"
    "<body>\n"
    "<svg viewBox='0 0 13330 10000' style='display:block; position:absolute; top:5%; left:5%; width:90%; height:90%;'>\n"
    "<defs>\n"
    "<g id='hmark'>\n"
    "<path d='M0 310 l-50 -50 v-520 l50 -50 l50 50 v520 Z' class='indicator'>\n"
    "</g>\n"
    "<g id='xmark'>\n"
    "<path d='M0 220 l-150 150 h300 Z ' class='indicator'>\n"
    "</g>\n"
    "<g id='vmark'>\n"
    "<path d='M310 0 l-50 -50 h-520 l-50 50 l50 50 h520 Z' class='indicator'>\n"
    "</g>\n"
    "</defs>\n"
    "<g id='dyn'></g>\n"
    "</svg>\n"
    "</body>\n"
  "</html>\n"
  ));
  flush();
}

#include <SPI.h>            // required for ethernet2
#include <Ethernet2.h>
#include <string.h>
#include <avr/pgmspace.h>   // storing strings in Flash to save RAM

#define N(x) sizeof(x)/sizeof(x[0])
#include "gauge.h"
#include "config.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 111);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
EthernetClient client;

char buffer[64];
int bfree = 64;
char *bufp = buffer;

void flush() {
  client.write(buffer, 64-bfree);
  bfree = 64;
  bufp = buffer;
}

void print(string cp, int len) {
  while (len) {
    int n = len;
    if (n > bfree)
      n = bfree;
    memcpy(bufp, cp, n);
    bufp += n;
    bfree -= n;
    if (bfree == 0)
      flush();
    cp += n;
    len -= n;     
  }
}

void print_P(const __FlashStringHelper * cp, int len) {
  PGM_P pp = reinterpret_cast<PGM_P>(cp);
  while (len) {
    int n = len;
    if (n > bfree)
      n = bfree;
    memcpy_P(bufp, pp, n);
    bufp += n;
    bfree -= n;
    if (bfree == 0)
      flush();
    pp += n;
    len -= n;     
  }
}

void print(string cp) {
  print(cp, strlen(cp));
}

void print_P(const __FlashStringHelper * cp) {
  PGM_P pp = reinterpret_cast<PGM_P>(cp);
  print_P(cp, strlen_P(pp));
}

void print(char c) {
  print(&c,1);
}

void print(int n, int decimal) {
  char buf[7];
  buf[6] = 0;
  char *cp = buf + 6;
  if (n < 0) {
    print('-');
    n = -n;
  }
  do {
    *--cp = n % 10 + '0';
    n /= 10;
    if (decimal-- == 1)
      *--cp = '.';
  } while (n > 0 || decimal >= 0);
  print(cp);
}

void print(int n) {
  print(n,0);
}

int readPin(int p) {
  return rand() & 0x3ff;
  // if (p < 16)
  //  return analogRead(p);
  // get latest reading from thermocouple buffer
  // return thermocouple[p-16];
}






void print_n_close() {
  print_P(F("'/>\n"));
}

void print_n_close(int n) {
  print(n);
  print_n_close();
}

void print_g_close() {
  print_P(F("</g>\n"));
}

void print_text_close() {
  print_P(F("</text>\n"));
}

void print_text_close(string cp) {
  print(cp);
  print_text_close();
}


int scaleMark(const Sensor *s, int val) {
  int mark = (int)(((long)s->mfactor * (long)val) >> divisor) + s->moffset;
  if (mark < 0)
    mark = 0;
  if (mark > 1000)
    mark = 1000;
  return mark;
}

int scaleValue(const Sensor *s, int val) {
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





void serveUp(char url) {
    unsigned long start = millis();         
   switch(url) {  
    case ' ':
      printPrefix();
      break;
    case 'd':    
      for (int i=0; i<N(gauges); i++)
        printGauge(gauges+i);
      flush();
      break;

  }
  Serial.print("Load page /");
  Serial.print(url);
  Serial.print(' ');
  Serial.print(int(millis()-start));
  Serial.print("ms\n");
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  delay(1000);    // replace this with LED self test sequence
//  pause for Serial Monitor to be started
//  while (!Serial) 
//    ; // wait for serial port to connect. Needed for native USB port only

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  const char *request = "GET /";
   // listen for incoming clients
  client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    char *state = request;
    char url = ' ';
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\r')
          continue;
        if (*state == 0) {
          url = c;
          state = request;
        }
          
        // Need to look for GET /x, where x is the webpage being loaded (or ' ' for the main page) !!!! 
        // ignore '/r', walk through string pointing to "GET /" called state, when *state == 0 then
        // use the next character to figure out which web page to load.
        // Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          // client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          serveUp(url);   
          break;
        }
        if (c && c == *state)
          *state ++;
        else
          state = request;

        currentLineIsBlank = (c == '\n');
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

// Uncommenting the following line shows a box around each instrumment and around the viewable area of the page. Use to help arrange gauges.
// #define BOUNDING_BOX

const char *green = "green";
const char *yellow = "yellow";
const char *red = "red";

#define divisor 13  // 1<<13 = 8192, this allows factors between -2.0 to 1.999
                    // for the 0-1023 ADC values, multiply range by 8 for full scale

#define MX(x)       (int)((x)*(1<<divisor) + 0.5)
#define GMX(range)  (int)(1000.0 / (range) * (1<<divisor) + 0.5)
#define GB(low)     (int)(-((low) + 0.5))  
#define ADCtoDivV   (5.0/1023.0)      // multiply this by adc input to get DC volts
#define ADCtoV10    (40 * ADCtoDivV)  // 1:4 voltage divider  
#define V10toADC    (1/(40 * ADCtoDivV))  // 1:4 voltage divide  

// Sensor defintions and scaling
// -----------------------------
// The sensors numeric values, y in the following forumala, are scaled as follow: y = m * x + b.
// Sensors gauge position are scaled a bit differerently with this formuala: y = (m + b) * x
// All of these numbers must be integers. Factors are represented as a ratio with an integer numerator and a denominator of 8192.
// Some numberic values are multipled by 10 and then have a decimal point added when displayed. 
// The MX macro converts a floating point factor for m into an integer factor. The gauge pointers vary from 0 to 1000. 
// Use GB() and GMX to set the graphs b and m values based on lowest input and input range (hi-low) respectively. 

//                   sensor-type,  pin, decimal, voffset,      vfactor,          moffset,          mfactor, lowAlarm, lowAlert, highAlert, highAlarm
const Sensor vtS = { st_volts,       0,       1,       0, MX(ADCtoV10), GB(100*V10toADC), GMX(60*V10toADC),      110,      130,      9999,       160 };
const Sensor opS = { st_r240to33,    1,       0,       0,      MX(0.1),                0,          MX(1.0),       25,       55,      9999,        95 }; 
const Sensor otS = { st_thermistorF, 2,       0,       0,       MX(.1),        GB(50*10),      GMX(200*10),       -1,      140,      9999,       250 };
const Sensor fpS = { st_r240to33,    3,       1,       0,      MX(0.1),                0,          MX(1.0),        5,       20,        60,        80 };
const Sensor flS = { st_r240to33,    4,       1,       0,     MX(0.16),                0,          MX(1.0),       25,       50,      9999,       999 };
const Sensor taS = { st_tachometer, 15,       0,       0,       MX(1.),                0,        GMX(3000),       -1,      500,      9999,      2700 };
const Sensor maS = { st_volts,      -1,       1,     100,         2000,                0,             8008,       -1,       -1,      9999,      9999 }; 
const Sensor chS = { st_k_type_tcF, 16,       0,       0,      MX(.25),        GB(100*4),       GMX(400*4),       -1,      150,       400,       500 };  
const Sensor egS = { st_k_type_tcF, 20,       0,       0,      MX(.25),       GB(1000*4),       GMX(600*4),       -1,       -1,      9999,      9999 };

// Labels
// ------
string opLV[] =   { "80", "60", "40", "20"  };
const int opLP[] =  { 800,  1600, 2400, 3200 };
string otLV[] =   { "200",  "150",  "100" };
const int otLP[] =  { 1000, 2000, 3000 };
string vtLV[] =   { "14", "12"  };
const int vtLP[] =  { 1330, 2670 };
string fpLV[] =   { "8",  "6",  "4",  "2" };
const int fpLP[] =  { 800,  1600, 2400, 3200 };
string flLV[] =   { "15",  "10",  "5",  "2.5" };
const int flLP[] =  { 250,  1500, 2750, 3375 };
string chLV[] =   { "200&deg;F",  "300&deg;F",  "400&deg;F" };
const int chLP[] =  { 2000,   4000, 6000 };
string egLV[] =   { "1150&deg;",  "1300&deg;",  "1450&deg;" };
const int egLP[] =  { 2000,   4000, 6000 };

// Color regions
// -------------
string opRC[] = { red,    yellow, green,  red     };
int    opRP[] = { 1000,   2200,   3800,   4000      };
string otRC[] = { yellow, green,  red         };
int    otRP[] = { 1800,   3925,   4000          };
string vtRC[] = { red,    yellow, green,  yellow, red };
int    vtRP[] = { 700,    2000,   3330,   3925,   4000  };
string fpRC[] = { red,    yellow, green,  yellow, red };
int    fpRP[] = { 200,    800,    2400,   3200,   4000  };
string flRC[] = { red,    yellow, green         };
int    flRP[] = { 625,    1250,   4000          };
string taRC[] = { yellow,    green,   red       };
int    taRP[] = { -1280,  1292,   1126,         // x
                  -238,   124,    650,          // y
                   0,    0,      0       };     // > 180 degrees
string maRC[] = { green                 };
int    maRP[] = { 1126,                     // x
                  650,                      // y
                    1                 };    // > 180 degrees
string chRC[] = { yellow, green, yellow,  red     };
int    chRP[] = { 1000,   6000,   7940,   8000    };

// Gauge layout for screen
// -----------------------
#define bank 3500   // bank of misc vertical gauges
const Gauge gauges[] = {
  //  x,      y,  style,     label1, label2, units,  labVal,  labPt,     num, regClr, regPt,     num, sensor
  {bank+0,    0,  gs_vert,   "OIL",  "PRES", "psi",    opLV,   opLP, N(opLV),   opRC,  opRP, N(opRC),  &opS},
  {bank+1750, 0,  gs_vert,   "OIL",  "TEMP", "&deg;F", otLV,   otLP, N(otLV),   otRC,  otRP, N(otRC),  &otS},
  {bank+3500, 0,  gs_vert,   "",     "VOLT", "volt",   vtLV,   vtLP, N(vtLV),   vtRC,  vtRP, N(vtRC),  &vtS},
  {bank+5250, 0,  gs_vert,   "FUEL", "PRES", "psi",    fpLV,   fpLP, N(fpLV),   fpRC,  fpRP, N(fpRC),  &fpS},
  {bank+7000, 0,  gs_pair,   "FUEL", "",     "gal",    flLV,   flLP, N(flLV),   flRC,  flRP, N(flRC),  &flS}, 
  {100,       0,  gs_round,  "TACH", "",     "rpm",    0,      0,    0,         taRC,  taRP, N(taRC),  &taS},
  {100,    3200,  gs_round,  "MP",   "",     "in-hg",  0,      0,    0,         maRC,  maRP, N(maRC),  &maS},
  {2950,    6150, gs_horiz,  "CHT",  "",     "",       chLV,   chLP, N(chLV),   chRC,  chRP, N(chRC),  &chS},
  {2950,    6150, gs_aux,    "EGT",  "",     "",       egLV,   egLP, N(egLV),      0,     0,       0,  &egS},
  {800,     6650, gs_infobox,"",     "",     "",       0,      0,    0,            0,     0,       0,     0}
};

// Guage layout for aux display
#define XTEXT(a,b,c,d) { .literal = {LED_TEXT(a,b,c,d)} }   // literals must not be blank in last 2 letters
#define XSENSOR(x)     { .sensor = {&x, 0} }    
const AuxDisplay auxdisplay[] = {
  XTEXT( ,b,A,t), XSENSOR(vtS),  
  XSENSOR(taS),   XSENSOR(flS),
  XTEXT( , ,O,P), XSENSOR(opS),  
  XTEXT( , ,O,t), XSENSOR(otS),  
  XTEXT( , ,F,P), XSENSOR(fpS),  
  XTEXT( ,A,L,t), XSENSOR(vtS),  
};


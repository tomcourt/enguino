// Uncommenting the following line shows a box around each instrumment and around the viewable area of the page. Use to help arrange gauges.
// #define BOUNDING_BOX

#define TACH_DIVIDER 4

const char *green = "green";
const char *yellow = "yellow";
const char *red = "red";

#define divisor 13  // 1<<13 = 8192

#define SCALE(factor)   (int)((factor)*(1<<divisor) + 0.5)  // Converts a floating point factor to integer
#define GRNG(range)     SCALE(1000.0/(range))               // Used for gfactor, given range returns gfactor that will result in 0-1000 
#define GMIN(low)       (int)(-((low) + 0.5))  
#define ADCtoV          (5.0/1024.0)                        // multiply this by adc input to get DC volts
#define toV             (40 * ADCtoV)                       // 1:4 voltage divider, V/10 = adc*toV  
#define fromV           (1/toV)                             // 1:4 voltage divide, (V/10) * fromV = adc  
// For Van's gauges or other 240-33 ohm gauges that have a linear voltage response instead of linear resistance use st_volts with the following: 
#define RVoff           -495                    // 495 is when ADC when gauge reads 0
#define RVscale         (1000.0/(124+RVoff))    // 124 is when ADC when gauge reads max
#define RVSCALE(factor) SCALE((factor)*RVscale)
#define RVRNG(range)    SCALE(RVscale*(range)/1000.0)

// Sensor defintions and scaling
// -----------------------------
// The sensors are scaled using the following formula, are scaled as follow: y = factor * (adc + offset).
// All of these numbers must be integers. Factors are represented as a ratio with an integer numerator and a denominator of 8192.
// Some numeric values are multipled by 10 and then have a decimal point added when displayed. 
// SCALE(1.) returns the raw values (ADC, RPM's). SCALE(range/full_scale) returns a value from 0 to range.
// The gauge pointers vary from 0 to 1000. Use GMIN() and GRNG to set the graphs b and m values based on lowest input and input range (hi-low) respectively. 

//   sensor-type    range   
// --------------  --------
// st_r240to33     0 - 1000   proportional resistive sensor
// st_thermistorC  0 - 1500   degrees C. in tenths
// st_thermistorF 32 - 2732   degrees F. in tenths
// st_volts        0 - 1023   ADC units 4.88 mV/per
// st_k_type_tcC   0 - 4000   0-1000 degrees C. in quarters
// st_j_type_tcC   0 - 4000   0-1000 degrees C. in quarters
// st_k_type_tcF   0 - 4000   32-1832 degrees F. in quarters
// st_j_type_tcF   0 - 4000   32-1832 degrees F. in quarters
// st_tachometer
// st_fuel_flow

//                   sensor-type,  pin, decimal, voffset,         vfactor,            goffset,           gfactor, lowAlarm, lowAlert, highAlert, highAlarm
const Sensor vtS = { st_volts,       0,       1,       0, SCALE(200/1024.0),  GMIN(100*fromV),    GRNG(60*fromV),      110,      130,      9999,       160 };
const Sensor opS = { st_volts,       1,       0,   RVoff,      RVRNG(100),              RVoff,       RVSCALE(1.),       25,       55,      9999,        95 }; 
const Sensor otS = { st_thermistorF, 2,       0,       0,       SCALE(.1),        GMIN(50*10),      GRNG(200*10),       -1,      140,      9999,       250 };
const Sensor fpS = { st_volts,       3,       1,   RVoff,      RVRNG(150),              RVoff,      RVSCALE(1.5),        5,       20,        60,        80 };
const Sensor flS = { st_volts,       4,       1,   RVoff,      RVRNG(160),              RVoff,       RVSCALE(1.),       25,       50,      9999,       999 };
const Sensor taS = { st_tachometer, 15,       0,       0,       SCALE(1.),                  0,        GRNG(3000),       -1,      500,      9999,      2700 };
const Sensor maS = { st_volts,      -1,       1,     100,            2000,                  0,         SCALE(1.),       -1,       -1,      9999,      9999 }; 
const Sensor chS = { st_k_type_tcF, 16,       0,       0,      SCALE(.25),        GMIN(100*4),       GRNG(400*4),       -1,      150,       400,       500 };  
const Sensor egS = { st_k_type_tcF, 20,       0,       0,      SCALE(.25),       GMIN(1000*4),       GRNG(600*4),       -1,       -1,      9999,      9999 };

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
// Change layout to literal, sensor, sensor.  Where a sensor value is displayed in the top line if literal is blank, otherwise sensor is used for alarm state of top line
#define XTEXT(a,b,c,d) { .literal = {LED_TEXT(a,b,c,d)} }   // literals must have characters in one of the last 2 letters
#define XSENSOR(x)     { .sensor = {&x, 0} }    
const AuxDisplay auxdisplay[] = {
  XTEXT( ,b,A,t), XSENSOR(vtS),  
  XSENSOR(taS),   XSENSOR(flS),
  XTEXT( , ,O,P), XSENSOR(opS),  
  XTEXT( , ,O,t), XSENSOR(otS),  
  XTEXT( , ,F,P), XSENSOR(fpS),  
  XTEXT( ,A,L,t), XSENSOR(vtS),  
};


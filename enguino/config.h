const char *green = "green";
const char *yellow = "yellow";
const char *red = "red";

#define divisor 13  // 1<<13 = 8192, this allows factors between -2.0 to 1.999
                    // for the 0-1023 ADC values, multiply range by 8 for full scale

#define MX(x)       (int)((x)*(1<<divisor) + 0.5)
#define GMX(range)  (int)(1000.0 / (range) * (1<<divisor) + 0.5)
#define GB(low)     (int)(-((low) + 0.5))  
#define ADCtoV  (5.0/1023.0)    // multiply this by divider to get DC volts

const int thermistorADC[]  = {};
const int thermistorC10[]  = {};

const int resist240ADC[]   = {};
const int resist240p1000[] = {};

// Sensor defintions and scaling
// -----------------------------
// Sensors are scaled using the formula: y = m * x + b, where y is either numeric value displayed or the gauge pointer postion.
// All of these numbers must be integers. Factors are represented as a ratio with an integer numerator and a denominator of 8192.
// Some numberic values are multipled by 10 and then have a decimal point added when displayed. 
// The MX macro converts a floating point factor for m into an integer factor. The gauge pointers vary from 0 to 1000. 
// Use GB() and GMX to set the graphs b and m values based on lowest input and input range (hi-low) respectively. 
//
const Sensor opS = {    st_r240to33,       0,       MX(0.1),                    0,  MX(1.0)};     // 0 - 100
const Sensor otS = {    st_r240to33,      50,       MX(0.2),                    0,  MX(1.0)};     // 50 - 250
const Sensor vtS = {    st_volts,          0, MX(ADCtoV*40),                    0,  8008};        // 100 - 160   (10 - 16)
const Sensor fpS = {    st_r240to33,       0,       MX(0.1),                    0,  MX(1.0)};     // 0 - 100     (0 - 10)
const Sensor flS = {    st_r240to33,       0,      MX(0.16),                    0,  MX(1.0)};     // 0 - 160     (0 - 16)
const Sensor taS = {    st_tachometer,     0,         12000,                    0,  8008 };       // 0 - 3000 
const Sensor maS = {    st_volts,        100,          2000,                    0,  8008};        // 100 - 350   (10 - 35)
const Sensor chS = {    st_thermocouple,  32,     MX(1.8/4),  GB((100-32)*5./9.*4), GMX(400*5./9.*4)}; // 100 - 500,  input is .25 C, convert to whole F   
const Sensor egS = {    st_thermocouple,  32,     MX(1.8/4), GB((1000-32)*5./9.*4), GMX(600*5./9.*4)}; // 1000 - 1600,input is .25 C, convert to to whole F

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
int    otRP[] = { 1800,   3900,   4000          };
string vtRC[] = { red,    yellow, green,  yellow, red };
int    vtRP[] = { 700,    2000,   3330,   3900,   4000  };
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
  //  x,      y,  style,  decimal,  label1, label2, units,  labVal, labPt,num,    regClr, regPt, num,    sensor,  pin
  {bank+0,    0,  gs_vert,  0,    "OIL",  "PRES", "psi",    opLV,   opLP, N(opLV),  opRC, opRP, N(opRC),  &opS,   0},
  {bank+1750, 0,  gs_vert,  0,    "OIL",  "TEMP", "&deg;F", otLV,   otLP, N(otLV),  otRC, otRP, N(otRC),  &otS,   1},
  {bank+3500, 0,  gs_vert,  1,    "",     "VOLT", "volt",   vtLV,   vtLP, N(vtLV),  vtRC, vtRP, N(vtRC),  &vtS,   2},
  {bank+5250, 0,  gs_vert,  1,    "FUEL", "PRES", "psi",    fpLV,   fpLP, N(fpLV),  fpRC, fpRP, N(fpRC),  &fpS,   3},
  {bank+7000, 0,  gs_pair,  1,    "FUEL", "",     "gal",    flLV,   flLP, N(flLV),  flRC, flRP, N(flRC),  &flS,   4}, // pins 4 and 5
  {100,       0,  gs_round, 0,    "TACH", "",     "rpm",    0,      0,    0,        taRC, taRP, N(taRC),  &taS,   -1},
  {100,    3200,  gs_round, 1,    "MP",   "",     "in-hg",  0,      0,    0,        maRC, maRP, N(maRC),  &maS,   -1},
  {3000,    6250, gs_horiz, 0,    "CHT",  "",     "",       chLV,   chLP, N(chLV),  chRC, chRP, N(chRC),  &chS,   16},
  {3000,    6250, gs_aux,   0,    "EGT",  "",     "",       egLV,   egLP, N(egLV),  0,    0,    0,        &egS,   20}
};


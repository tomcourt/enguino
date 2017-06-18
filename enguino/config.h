const char *green = "green";
const char *yellow = "yellow";
const char *red = "red";

#define divisor 13  // 1<<13 = 8192, this allows factors between -2.0 to 1.999
                    // for the 0-1023 ADC values, multiply range by 8 for full scale
const Sensor opS = {    st_volts,   0,   800,  0,  8008};    // 0 - 100
const Sensor otS = {    st_volts,   50,  400,  0,  8008};    // 50 - 250
const Sensor vtS = {    st_volts,   100, 4000,  0, 8008};    // 100 - 600  (10 - 16)
const Sensor fpS = {    st_volts,   0,   800,  0,  8008};    // 0 - 100    (0 - 10)
const Sensor flS = {    st_volts,   0,   1280,  0, 8008};    // 0 - 160    (0 - 16)
const Sensor taS = {    st_volts,   0,  12000,  0,  8008 };  // 0 - 3000 
const Sensor maS = {    st_volts,   100, 2000,  0,  8008};   // 100 - 350   (10 - 35)
const Sensor chS = {    st_volts,   100, 3200,  0,  8008};   // 100 - 500,  input is .25 C, convert to whole F   
const Sensor egS = {    st_volts,   1000,4800,  0,  8008};   // 1000 - 1600,  input is .25 C, convert to to whole F

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


#define bank 3500   // bank of misc vertical gauges
const Gauge gauges[] = {
  //  x,      y,  style,  decimal,  label1, label2, units,  labVal, labPt,num,    regClr, regPt, num,    sensor,  pin
  {bank+0,    0,  gs_vert,  0,    "OIL",  "PRES", "psi",    opLV,   opLP, N(opLV),  opRC, opRP, N(opRC),  &opS,   0},
  {bank+1750, 0,  gs_vert,  0,    "OIL",  "TEMP", "&deg;F", otLV,   otLP, N(otLV),  otRC, otRP, N(otRC),  &otS,   1},
  {bank+3500, 0,  gs_vert,  1,    "",     "VOLT", "volt",   vtLV,   vtLP, N(vtLV),  vtRC, vtRP, N(vtRC),  &vtS,   2},
  {bank+5250, 0,  gs_vert,  1,    "FUEL", "PRES", "psi",    fpLV,   fpLP, N(fpLV),  fpRC, fpRP, N(fpRC),  &fpS,   3},
  {bank+7000, 0,  gs_pair,  1,    "FUEL", "",     "gal",    flLV,   flLP, N(flLV),  flRC, flRP, N(flRC),  &flS,   4}, // pins 4 and 5
  {100,       0,  gs_round, 0,    "TACH", "",     "rpm",    0,      0,    0,        taRC, taRP, N(taRC),  &taS,   6},
  {100,    3200,  gs_round, 1,    "MP",   "",     "in-hg",  0,      0,    0,        maRC, maRP, N(maRC),  &maS,   7},
  {3000,    6250, gs_horiz, 0,    "CHT",  "",     "",       chLV,   chLP, N(chLV),  chRC, chRP, N(chRC),  &chS,   16},
  {3000,    6250, gs_aux,   0,    "EGT",  "",     "",       egLV,   egLP, N(egLV),  0,    0,    0,        &egS,   20}
};


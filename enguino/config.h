// Uncommenting the following line shows a box around each instrumment and around the viewable area of the page. Use to help arrange gauges.
// #define BOUNDING_BOX

#define TACH_DIVIDER 4

// Exceed any of these and engine will be considered 'running'. Hobbs time will accumulate and engine alerts will appear. Setting to 0 will ignore that sensor
#define RUN_VOLT 130
#define RUN_OILP 10
#define RUN_TACH 200

// Sensor defintions and scaling
// -----------------------------
// The sensors are scaled using the following formula, are scaled as follow: y = factor * (adc + offset).
// All of these numbers must be integers. Factors are represented as a ratio with an integer numerator and a denominator of 8192.
// Some numeric values are multipled by 10 and then have a decimal point added when displayed. 
// SCALE(1.) returns the raw values (ADC, RPM's). SCALE(range/full_scale) returns a value from 0 to range.
// The gauge pointers vary from 0 to 1000. Use GMIN() and GRNG to set the graphs b and m values based on lowest input and input range (hi-low) respectively. 
// Manifold pressure sensor = P(inch) = V(full) * 32.811 + 3.117 or V = (P-3.117)/32.811

//   sensor-type    range   
// --------------  --------
// st_r240to33     0 - 1000   proportional on resistance of sensor forming resistor divider
// st_v240to33     0 - 1000   proportional on voltage of sensor forming resistor divider
// st_thermistorC  0 - 1500   degrees C. in tenths
// st_thermistorF 32 - 2732   degrees F. in tenths
// st_volts        0 - 1000   5 mV/per count
// st_k_type_tcC   0 - 4000   0-1000 degrees C. in quarters
// st_j_type_tcC   0 - 4000   0-1000 degrees C. in quarters
// st_k_type_tcF   0 - 4000   32-1832 degrees F. in quarters
// st_j_type_tcF   0 - 4000   32-1832 degrees F. in quarters
// st_tachometer
// st_fuel_flow

//                      sensor-type,  pin, decimal, voffset,         vfactor,            goffset,          gfactor,lowWarning,lowCaution,highCaution,highWarning
const Sensor voltS =  { st_volts,       0,       1,       0,     SCALE(.200),    GMIN(100*fromV),    GRNG(60*fromV),      110,      130,      9999,       160 };
const Sensor oilpS =  { st_v240to33,    1,       0,       0,     SCALE(.100),                  0,        SCALE(1.),       25,       55,      9999,        95 }; 
const Sensor oiltS =  { st_thermistorF, 2,       0,       0,     SCALE(.100),        GMIN(50*10),      GRNG(200*10),       -1,      140,      9999,       250 };
const Sensor fuelpS = { st_v240to33,    3,       1,       0,     SCALE(.150),                  0,  SCALE(150./100.),        5,       20,        60,        80 };
const Sensor fuellS = { st_v240to33,   DUAL(4),  1,       0,     SCALE(.160),                  0,         SCALE(1.),       25,       50,      9999,       999 };    
const Sensor tachS =  { st_tachometer, 15,       0,       0,       SCALE(1.),            GMIN(0),        GRNG(3000),       -1,      500,      9999,      2700 };
const Sensor mapS =   { st_volts,      -1,       1,      31,   SCALE(.32811),          GMIN(210),   GRNG(25/32.811),       -1,       -1,      9999,      9999 }; 
const Sensor chtS =   { st_k_type_tcF, 16,       0,       0,      SCALE(.25),        GMIN(100*4),       GRNG(400*4),       -1,      150,       400,       500 };  
const Sensor egtS =   { st_k_type_tcF, 20,       0,       0,      SCALE(.25),       GMIN(1000*4),       GRNG(600*4),       -1,       -1,      9999,      9999 };

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
int    taRP[] = { ARCX(5./30.)-1,  ARCX(27./30.),    ARCX(1),     // x  (-1 tweak on the arc to get green arc to overlay black arc well, small truncation here causes big arc error)
                  ARCY(5./30.),    ARCY(27./30.),    ARCY(1),     // y 
                  0,               0,                0       };   // > 180 degrees
string maRC[] = { green                 };
int    maRP[] = { ARCX(1),                  // x
                  ARCY(1),                  // y
                    1                 };     // > 180 degrees
string chRC[] = { yellow, green, yellow,  red     };
int    chRP[] = { 1000,   6000,   7940,   8000    };

// Gauge layout for screen
// -----------------------
#define bank 3500   // bank of misc vertical gauges
const Gauge gauges[] = {
  //  x,      y,  style,     label1, label2, units,  labVal,  labPt,     num, regClr, regPt,     num, sensor
  {bank+0,    0,  gs_vert,   "OIL",  "PRES", "psi",    opLV,   opLP, N(opLV),   opRC,  opRP, N(opRC),  &oilpS},
  {bank+1750, 0,  gs_vert,   "OIL",  "TEMP", "&deg;F", otLV,   otLP, N(otLV),   otRC,  otRP, N(otRC),  &oiltS},
  {bank+3500, 0,  gs_vert,   "",     "VOLT", "volt",   vtLV,   vtLP, N(vtLV),   vtRC,  vtRP, N(vtRC),  &voltS},
  {bank+5250, 0,  gs_vert,   "FUEL", "PRES", "psi",    fpLV,   fpLP, N(fpLV),   fpRC,  fpRP, N(fpRC),  &fuelpS},
  {bank+7000, 0,  gs_pair,   "FUEL", "",     "gal",    flLV,   flLP, N(flLV),   flRC,  flRP, N(flRC),  &fuellS}, 
  {100,       0,  gs_round,  "TACH", "",     "rpm",    0,      0,    0,         taRC,  taRP, N(taRC),  &tachS},
  {100,    3200,  gs_round,  "MP",   "",     "in-hg",  0,      0,    0,         maRC,  maRP, N(maRC),  &mapS},
  {2950,    6150, gs_horiz,  "CHT",  "",     "",       chLV,   chLP, N(chLV),   chRC,  chRP, N(chRC),  &chtS},
  {2950,    6150, gs_aux,    "EGT",  "",     "",       egLV,   egLP, N(egLV),      0,     0,       0,  &egtS},
  {800,     6650, gs_infobox,"",     "",     "",       0,      0,    0,            0,     0,       0,     0}
};

// Guage layout for aux display
// ----------------------------
// Layout is text, top-sensor, bottom-sensor. Sensor value is displayed in the top line if text is blank, otherwise top sensor is used to show warning/caution and low/high on top line
AuxDisplay auxDisplay[] = {
  AUX(b,A,t, , voltS,  voltS),  
  AUX( , , , , tachS,  fuellS),
  AUX(O,P, , , oilpS,  oilpS),  
  AUX(O,t, , , oiltS,  oiltS),  
  AUX(F,P, , , fuelpS,  fuelpS),  
  AUX(A,L,t, , voltS,  voltS),  
};

#define AUX_STARTUP_PAGES 1


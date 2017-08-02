// Copyright 2017, Thomas Court
//
// Typedes and defines for config.h
// --------------------------------
// Sketches don't like typdef's so they are in in this header file instead
//
//  This file is part of Enguino.
//  
//  Enguino is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  Enguino is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with Enguino.  If not, see <http://www.gnu.org/licenses/>.




#define N(x) sizeof(x)/sizeof(x[0])

#define FAULT -32768

typedef const char * string;

// for persist.h
// -------------
// both of these must be exactly 8 bytes in size
typedef struct  {
  word fullFuel;
  word kFactor;  // counts per 1/40 gallon
  word filler1;
  word filler2;
} EESettings;

typedef struct {
  byte sequence;
  byte hobbs1k; // thousands of an hour rollover
  word hobbs;   // hobbs in 1/40 of an hour 0-999.975 (0-39,999)
  word fuel;    // fuel remaining in 1/40 of a gallon (10 GPH, changes every 9 seconds)
  word filler;
} EEStatus;

// for sensor.h
// ------------
typedef struct {
  int start;
  byte n;
  const byte *log2diff;
  const int  *result;
} InterpolateTable;

enum SensorType {st_r240to33, st_v240to33, st_thermistorF, st_thermistorC, st_volts, st_k_type_tcF, st_j_type_tcF, st_k_type_tcC, st_j_type_tcC, st_unit};

typedef struct {
  SensorType type;    
  signed char pin;
  byte decimal;   // add decimal point 'decimal' positons from the right (0 is integer)
  int voffset;    // used to display reading, int_reading has 'decimal' point shifted right
  int vfactor;    // int_reading = multiply * sensor >> divisor + offset
  int goffset;    // used to calculate gauge marker position
  int gfactor;    // 0-4000 vertical gauge, 0-8000 horizontal gauge, 0-2400 round gauge
  int lowWarning;
  int lowCaution;
  int highCaution;
  int highWarning;
} Sensor;

enum GaugeStyle { gs_vert, gs_pair, gs_round, gs_horiz, gs_egt, gs_infobox };

typedef struct {
  int x;
  int y;
  GaugeStyle style;
  
  string label1;
  string label2;
  string units;
  
  string *labelValues;
  const int *labelPts;    // prescaled from low,high and unscaled pts
  byte n_labels;
  
  string *regionColors;
  const int *regionEndPts;  // prescaled from low,high and unscaled pts
  byte n_regions;
  
  const Sensor *sensor;
} Gauge;

enum GaugeColor { gc_green, gc_yellow, gc_red };

#define TACH_SENSOR   15
#define FUELF_SENSOR  14
#define FUELR_SENSOR  13
#define HOBBS_SENSOR  12

// alert states
#define WARNING_LOW    1
#define WARNING_HIGH   2
#define WARNING_ANY    3
#define CAUTION_LOW    4
#define CAUTION_HIGH   8
#define CAUTION_ANY    12
#define ALERT_FAULT    16

typedef struct {
  byte literal[4];
  Sensor *sensor[2];
  byte alertState[2];       
  signed char warning;     // +1)alert, -1)supress, 0)no alert
  signed char caution;     // +1)alert, -1)supress, 0)no alert
} AuxDisplay;


// definitions to support config.h
// -------------------------------
#define DUAL_BIT 64
#define DUAL(x)   (DUAL_BIT | (x))

// for a round gauge, calculate the x,y location for a gauge at fraction x of the full 240 degree arc
#define ARCX(x) (.5-(13000 * COS2(((x)*4./3.-1./6.) * PI)))
#define ARCY(x) (.5-(13000 * SIN2(((x)*4./3.-1./6.) * PI)))
#define VSEG(x) (.5+4000*(x))
#define HSEG(x) (.5+8000*(x))

#define AUX(a,b,c,d,top,bottom) {{LED_TEXT(a,b,c,d)},{top,bottom}}   

const char *green = "green";
const char *yellow = "yellow";
const char *red = "red";

#define divisor 13  // 1<<13 = 8192

#define SCALE(factor)   (int)((factor)*(1<<divisor) + 0.5)  // Converts a floating point factor to integer
#define GRNG(range)     SCALE(1000.0/(range))               // Used for gfactor, given range returns gfactor that will result in 0-1000 
#define GMIN(low)       (int)(-((low) + 0.5))  
#define ADCtoV          .005                                // 5 mV
#define toV             (40 * ADCtoV)                       // 1:4 voltage divider, V/10 = adc*toV  
#define fromV           (1/toV)                             // 1:4 voltage divide, (V/10) * fromV = adc  





// sketches don't like typdef's so they are in in this header file instead

#define N(x) sizeof(x)/sizeof(x[0])

#define FAULT -32768

typedef const char * string;

typedef struct {
  int start;
  byte n;
  const byte *log2diff;
  const int  *result;
} InterpolateTable;

enum SensorType {st_r240to33, st_thermistor, st_volts, st_k_type_tc, st_j_type_tc, st_tachometer, st_fuel_flow};
// st_r240to33 -     0 - 1000      proportional resistive sensor
// st_thermistor -   0 - 1500      degrees C. in tenths
// st_volts -        0 - 1023      ADC units 4.88 mV/per
// st_k_type_tc -    0 - 4000      0-1000 degrees C. in quarters
// st_j_type_tc -    0 - 4000      0-1000 degrees C. in quarters
// st_tachometer
// st_fuel_flow

// for K style in deg. C, use a multiply of   4096  (0.25),                       offset 0
// for K style in deg. F, use a multiplier of 7373  (0.25 * 1.8),                 offset -32
// for J style in deg. C, use a multiplier of 5751  (0.25 * 57.953/41.276),       offset 0
// for J style in deg. F, use a multiplier of 10352 (0.25 * 1.8 * 57.953/41.276), offset -32

typedef struct {
  SensorType type;    
  int voffset;    // used to display reading, int_reading has 'decimal' point shifted right
  int vfactor;    // int_reading = multiply * sensor >> divisor + offset
  int moffset;    // used to calculate marker position
  int mfactor;    // 0-4000 vertical gauge, 0-8000 horizontal gauge, 0-2400 round gauge
} Sensor;

enum GaugeStyle { gs_vert, gs_pair, gs_round, gs_horiz, gs_aux };

typedef struct {
  int x;
  int y;
  GaugeStyle style;
  
  int decimal;  // add decimal point 'decimal' positons from the right (0 is integer)
  
  string label1;
  string label2;
  string units;
  
  string *labelValues;
  const int *labelPts;    // prescaled from low,high and unscaled pts
  int n_labels;
  
  string *regionColors;
  const int *regionEndPts;  // prescaled from low,high and unscaled pts
  int n_regions;
  
  const Sensor *sensor;
  
  int pin;        // first pin if multiple sensors (cht/egt, fuel)
} Gauge;

enum GaugeColor { gc_green, gc_yellow, gc_red };



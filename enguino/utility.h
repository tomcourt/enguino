// compile time sin() and cos() for calculating round gauges position in an x, y plane
// https://github.com/pkhuong/polynomial-approximation-catalogue/tree/master/double
#define SIN(x)  ( (x) + -0.166656810730001*(x)*(x)*(x) + 0.008312366210465815*(x)*(x)*(x)*(x)*(x) + -1.8492181558254177e-4*(x)*(x)*(x)*(x)*(x)*(x)*(x) )
#define COS(x)  ( 1.0 + -0.4999356307314411*(x)*(x) + 0.04150706685139252*(x)*(x)*(x)*(x) + -0.0012757519849685426*(x)*(x)*(x)*(x)*(x)*(x)             )


void logTime(unsigned long start, const char *description) {
  Serial.print(description);
  Serial.print(' ');
  Serial.print(int(millis()-start));
  Serial.print("ms\n");
}

void logValue(int val, const char *description) {
  Serial.print(description);
  Serial.print('=');
  Serial.println(val);
 }

void logValue(int val, int num) {
  Serial.print(num);
  Serial.print('=');
  Serial.println(val);
}

void logText(const char *text = "") {
  Serial.println(text);
 }

// https://github.com/rekka/avrmultiplication
// signed16 * signed16
// 22 cycles
#define MultiS16X16to32(longRes, intIn1, intIn2) \
asm volatile ( \
"clr r26 \n\t" \
"mul %A1, %A2 \n\t" \
"movw %A0, r0 \n\t" \
"muls %B1, %B2 \n\t" \
"movw %C0, r0 \n\t" \
"mulsu %B2, %A1 \n\t" \
"sbc %D0, r26 \n\t" \
"add %B0, r0 \n\t" \
"adc %C0, r1 \n\t" \
"adc %D0, r26 \n\t" \
"mulsu %B1, %A2 \n\t" \
"sbc %D0, r26 \n\t" \
"add %B0, r0 \n\t" \
"adc %C0, r1 \n\t" \
"adc %D0, r26 \n\t" \
"clr r1 \n\t" \
: \
"=&r" (longRes) \
: \
"a" (intIn1), \
"a" (intIn2) \
: \
"r26" \
)

// multiply 16 x 16 signed values and return the 32 bit result
// this is a smaller and faster version of: (long(a) * long(b);
long multiply(int a, int b) {
  long res;

  MultiS16X16to32(res, a, b);
  return res;
}

int multiplyAndScale(int a, int b, byte shift) {
  return int(multiply(a, b) >> shift);
}

// interpolate values using table, returns FAULT instead of extrapolating
int interpolate(const InterpolateTable *table, int value) {
  int x = table->start;
  if (value < x)
    return FAULT;
  int i = table->n;
  byte *diff = table->log2diff;
  int  *result = table->result;
  for(;;) {
    if (value == x)
      return *result;
    if (--i == 0)
      return FAULT;
    int x1 = x + (1 << *diff);
    if (value < x1) 
      return int((multiply(*result,x1-value) + multiply(result[1], value-x)) >> *diff);
    x = x1;
    diff++;
    result++;
  }
}

// sort a small list using insertion sort O(n^2) worst case
void sort(int *list, int n) {
  for (byte i=1; i<n; i++) {
    for (byte j=i; j>0 && list[j-1] > list[j]; j--) {
      int t = list[j];
      list[j] = list[j-1];
      list[j-1] = t;
    }
  }
}


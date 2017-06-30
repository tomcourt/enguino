// lookup port mapping to pins here: https://www.arduino.cc/en/Reference/PortManipulation
#define SCL_PIN 7 
#define SCL_PORT PORTB 
#define SDA_PIN 0 
#define SDA_PORT PORTD
#define I2C_TIMEOUT 10 
#include "SoftI2CMaster.h"

#define HT16K33_OSCILATOR_ON 0x21

#define HT16K33_BLINK_CMD         0x80
#define HT16K33_BLINK_DISPLAYON   0x01
#define HT16K33_BLINK_OFF     (HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | 0)
#define HT16K33_BLINK_2HZ     (HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | 2)
#define HT16K33_BLINK_1HZ     (HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | 4)
#define HT16K33_BLINK_HALFHZ  (HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | 6)

#define HT16K33_CMD_BRIGHTNESS 0xE0
#define HT16K33_BRIGHT_MAX    (HT16K33_CMD_BRIGHTNESS | 15)
#define HT16K33_BRIGHT_MIN    (HT16K33_CMD_BRIGHTNESS | 1)

#define SEVENSEG_DIGITS 5
#define COLON_LIT       2

#define I2C_ADDRESS   0x70    // second line is 0x71

static const byte digittable[] = { 2, 4, 8, 10 };    // skip the colon 

// segments are aranged as follows (values are hexadecimal)
// +---  1 ---+
// |          |
// 20         2
// |          |   
// +--- 40 ---+
// |          |   
// 10         4
// |          |
// +---  8 ---+ *80

#define LED_TEXT(a,b,c,d) LED_ ## a, LED_ ## b, LED_ ## c, LED_ ## d

#define LED_0 0x3F
#define LED_1 0x06
#define LED_2 0x5B
#define LED_3 0x4F
#define LED_4 0x66
#define LED_5 0x6D
#define LED_6 0x7D
#define LED_7 0x07
#define LED_8 0x7F
#define LED_9 0x6F

#define LED_DP 0x80 // decimal point
#define LED__ 0x40  // minus sign
#define LED_COLON 2

#define LED_A 0x77
#define LED_b 0x7c
#define LED_C 0x39
#define LED_D 0x5e
#define LED_E 0x79
#define LED_F 0x71
#define LED_H 0x76
#define LED_h 0x74
#define LED_i 0x4
#define LED_L 0x38
#define LED_n 0x54
#define LED_o 0x5c
#define LED_P 0x73
#define LED_r 0x50
#define LED_t 0x78
#define LED_U 0x3e

static const byte numbertable[] = { LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7, LED_8, LED_9 };

static byte ledBuffer[11];   // first byte is 0 for the address, 2 is first digit, 4 the second, etc.

void writeI2C(byte line, byte *buffer, byte len) {
  if (!i2c_start((I2C_ADDRESS + line) | I2C_WRITE))
    return;
  while (len--) {
    if (!i2c_write(*buffer++)) 
      return false;   
  }
  i2c_stop();
}

// use this to set blink or brightness
void commandLED(byte line, byte command) {
  writeI2C(line, &command, 1);
}

void writeLED(byte line) {
  writeI2C(line, buffer, 11);  
}

void printLEDRawDigits(byte offset, word val) {
  while (--offset) {
    ledBuffer[digittable[offset]] = numbertable[val%10];
    val /= 10;
    if (val == 0)
      break;
  } 
}

void printLEDRawHalfDigits(byte offset, word number) {
  if (number == FAULT) {
    ledBuffer[digittable[offset-1]] = LED__;
    ledBuffer[digittable[offset-2]] = LED__;
  } 
  else {
    if (number < 0)
      number = 0;
    if (number > 999)
      number = 999;
    if (number < 100) {
      printLEDRawDigits(offset, number);
      ledBuffer[digittable[offset-2]] |= LED_DP;   // decimal point
    }
    else
      printLEDRawDigits(offset, number/10);
  }  
}

// -------------------------------------------------------

void printLEDSetup() {
  i2c_init();
  
  for (byte line=0; line<2; line++) {
    commandLED(line, HT16K33_OSCILATOR_ON);
    commandLED(line, HT16K33_BLINK_OFF);
    commandLED(line, HT16K33_BRIGHT_MAX);  
  }
}

// print a text message to the LED on line 0 or 1
void printLED(byte line, byte a, byte b, byte c, byte d) {
  ledBuffer[2] =  a;  
  ledBuffer[4] =  b;  
  ledBuffer[6] =  0; // turn colon off
  ledBuffer[8] =  c;  
  ledBuffer[10] = d;  
  writeLED(line);
}

// print a text message to the LED on line 0 or 1
void printLED(byte line, byte *txt) {
  ledBuffer[2] =  txt[0];  
  ledBuffer[4] =  txt[1];  
  ledBuffer[6] =  0; // turn colon off
  ledBuffer[8] =  txt[2];  
  ledBuffer[10] = txt[3];  
  writeLED(line);
}

// print the fuel gauge (e.g. 2.5:17)  (left tank : right tank)
void printLEDFuel(int left, int right) {
   memset(ledBuffer, 0, sizeof(ledBuffer));
   printLEDRawHalfDigits(4, right);
   printLEDRawHalfDigits(2, right);
   ledBuffer[6] = LED_COLON;    
   writeLED(1);
}

// print the 'number' to 'line' 0 or 1, place a decimal point 'decimal' digits to the left
void printLED(byte line, int number, byte decimal) {
  if (number == FAULT) {
    printLED(line, LED_TEXT(i,n,o,P));
  } 
  else {
    if (number < 0)
      number = 0;
    if (number > 9999)
      number = 9999;
    memset(ledBuffer, 0, 11);
    printLEDRawDigits(4, number);
    if (decimal != 0)
      ledBuffer[digittable[3-decimal]] |= LED_DP;
  }
  writeLED(line);
}


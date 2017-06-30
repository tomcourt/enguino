#include <EEPROM.h>

// both of these must be exactly 8 bytes in size
typedef struct  {
  word fullFuel;
  word kFactor;  // counts per 1/40 gallon
  word filler1;
  word filler2;
} EESettings;

typedef struct {
  byte sequence;
  byte filler;
  byte hobbs1k; // thousands of an hour rollover
  word hobbs;   // hobbs in 1/40 of an hour 0-999.975 (0-39,999)
  word fuel;    // fuel remaining in 1/40 of a gallon (10 GPH, changes every 9 seconds)
} EEStatus;

EESettings ee_settings;
EEStatus ee_status;

static byte nextSlot = 1;

// slot 0 is for settings, slots 1 through 63 is for status, status' storage slot is distributed for 'wear-leveling' 
// each slot has the data written twice, the second time is inverted
// if the first half doesn't match the inverted second half than return false (no valid data)
static bool eeRead(byte slot, void *buffer) {
  int address = slot << 4;
  byte *cp = buffer;
  for (byte i=8; i; i--)
    *cp++ = EEPROM.read(address++); 
  cp = buffer;
  for (byte i=8; i; i--)
    if (*cp++ != EEPROM.read(address++) ^ 0xFF) 
      return false;
  return true;  
}

// slot 0 is for settings, 1 through 63 is for status
// this takes 53 ms to complete
static void eeWrite(byte slot, void *buffer) {
  int address = slot << 4;
  byte *cp = buffer;
  for (byte i=8; i; i--)
    EEPROM.write(address++, *cp++); 
  cp = buffer;
  for (byte i=8; i; i--)
    EEPROM.write(address++, *cp++ ^ 0xFF) ;
}

// ----------------------------------------------------------

// only call in setup (assumes globals have been zeroed)
void eeInit() {
  if (!eeRead(0, &ee_settings)) {
    // ee_settings.fullFuel = 0; zeroed in initialization 
    ee_settings.kFactor = 1700;
    eeWrite(0, &ee_settings);
  }

  // search all the slots for the highest sequence number
  bool found = false;
  EEStatus temp;
  while (nextSlot++ < 64) {
    if (!eeRead(nextSlot, &temp))
      continue;
    // handle rollover math for sequence number
    if (found && (signed char)(temp.sequence - ee_status.sequence) <= 0)
      continue;
    ee_status = temp;
    found = true;
  }
  nextSlot = 2;
  // ee_status.ALL = 0; zeroed in initialization 
  eeWrite(1, (void *)&ee_status);  
}

// this takes 53 ms to complete
void eeUpdateSettings() {
  eeWrite(0, &ee_settings);
}

// this takes 53 ms to complete
// 15,000+ hour eeprom lifetime (at 10GPH) with updates whenever hobbs or gallons changes
void eeUpdateStatus() {
  ee_status.sequence++;
  eeWrite(0, &ee_status);
  if (++nextSlot == 0)
    nextSlot = 1;
}


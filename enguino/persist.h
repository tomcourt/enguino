// Persistant vairable storage
// ---------------------------
// Reads and writes 8 byte buffers to EEPROM.
// Buffers have 2 copies, the second copy is inverted. This provides for error dection.
// ee_settings is for variables that don't often change. This can be updated up to 10,000 times.
// ee_status is for variables that change often. It is written across many locations in EEPROM 
// for wear-leaveling. It can be updated up to 600,000 times. 

#include <EEPROM.h>

static byte nextFreeSlot;

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
    if (*cp++ != (EEPROM.read(address++) ^ 0xFF)) 
      return false;
  return true;  
}

// slot 0 is for settings, 1 through 63 is for status
// this takes 53 ms to complete
static void eeWrite(byte slot, void *buffer) {
  int address = slot << 4;
  
  byte copy[8];
  noInterrupts();
  memcpy(copy, buffer, sizeof(copy));
  interrupts();

  byte *cp = copy;
  for (byte i=8; i; i--)
    EEPROM.write(address++, *cp++); 
  cp = copy;
  for (byte i=8; i; i--)
    EEPROM.write(address++, *cp++ ^ 0xFF) ;
}

// ----------------------------------------------------------

// only call in setup (assumes globals have been zeroed)
void eeInit() {
  // EEPROM is preserved on Leanardo, to test fresh uncomment this
  //  for (int i = 0 ; i < EEPROM.length() ; i++) {
  //    EEPROM.write(i, 0);
  //  }
  
  if (!eeRead(0, &ee_settings)) {
    ee_settings.kFactor = DEFAULT_K_FACTOR;
    eeWrite(0, &ee_settings);
  }

  // search all the slots for the highest sequence number
  bool found = false;
  EEStatus temp;
  for (byte i=1; i<64; i++) { 
    if (!eeRead(i, &temp))
      continue;
    // handle rollover math for sequence number
    if (found && (signed char)(temp.sequence - ee_status.sequence) <= 0)
      continue;
    ee_status = temp;
    nextFreeSlot = i + 1;
    if (nextFreeSlot > 63)
      nextFreeSlot = 1;
    found = true;
  }
  if (found)
    return;
   // ee_status.ALL = 0; zeroed in initialization 
  eeWrite(1, (void *)&ee_status);  
  nextFreeSlot = 2;
}

// this takes 53 ms to complete
void eeUpdateSettings() {
  eeWrite(0, &ee_settings);
}

// this takes 53 ms to complete
// 15,000+ hour eeprom lifetime (at 10GPH) with updates whenever hobbs or gallons changes
void eeUpdateStatus() {
  ee_status.sequence++;
  eeWrite(nextFreeSlot, &ee_status);
  if (++nextFreeSlot > 63)
    nextFreeSlot = 1;
}


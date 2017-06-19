// Pinout for Ocean Design's Thermocouple Multiplexer Shield
#define PINEN 7 // TC Mux Enable pin
#define PINA0 4 // TC Mux Address 0 pin
#define PINA1 5 // TC Mux Address 1 pin
#define PINA2 6 // TC Mux Address 2 pin
#define PINSO 12 //TC ADC Slave Out pin (MISO)
#define PINSC 13 //TC ADC Serial Clock (SCK)
#define PINCS 9  //TC ADC Chip Select

volatile int tcTemp[9];    // in quarter deg. C, tcTemp[8] is the interal reference temp, disable IRQ's to access these

int readSPI() {
  word v = 0;
  for (byte i=16; i!=0; i--) {
    v <<= 1;
    digitalWrite(PINSC, HIGH);
    // 100nS min. delay implied
    v |= digitalRead(PINSO);
    digitalWrite(PINSC, LOW);   // request next serial bit
    // 100nS min. delay implied
  }
  return v;
}

void tcTempSetup() {
  pinMode(PINEN, OUTPUT);     
  pinMode(PINA0, OUTPUT);    
  pinMode(PINA1, OUTPUT);    
  pinMode(PINA2, OUTPUT);    
  pinMode(PINSO, INPUT);    
  pinMode(PINCS, OUTPUT);    
  pinMode(PINSC, OUTPUT);    
  
  digitalWrite(PINEN, HIGH);   // enable the mux all the time
  digitalWrite(PINSC, LOW);    // put clock in low
  digitalWrite(PINCS, LOW);    // stop conversion, start serial interface

  // Timer0's overflow is used for millis() - setup to interrupt
  // in the middle and call the "Compare A" function below
  OCR0A = 0x80;
  TIMSK0 |= _BV(OCIE0A);  
}

// Interrupt is called every millisecond
SIGNAL(TIMER0_COMPA_vect) 
{
  static byte ms = 0;
  static byte ch = 0;

  if (ms == 0) {
    // select the thermocouple channel on the mux
    digitalWrite(PINA0, ch&1); 
    digitalWrite(PINA1, ch&2); 
    digitalWrite(PINA2, ch&4);
    // ... wait a while for the capacitor on the ADC input to charge (< .1 mS actually needed)     
  }
  else if (ms == 5) {
    // begin conversion
    digitalWrite(PINCS, HIGH);
    // ... wait 100 mS for conversion to complete
  }
  else if (ms == 105) {
    // stop conversion, start serial interface
    digitalWrite(PINCS, LOW); 
    // 100nS min. delay implied 
    
    int rawTC = readSPI();
    int rawIT = readSPI();
    
    int tempC = rawTC / 4;
    if (rawTC & 1) {
      if (rawIT & 1)
        tempC = OPEN;
      if (rawIT & 6)
        tempC = SHORT;
    }
    tcTemp[ch] = tempC;

    if (++ch == 8) {
      tcTemp[8] = rawIT / 64; // internal temperature reduced to quarter degree C
      ch = 0;
    }
    ms = 255; // ++ will make this 0 
  }
  ms++;
}


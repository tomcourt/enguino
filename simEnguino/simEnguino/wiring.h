// Copyright 2017, Thomas Court
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

#ifndef wiring_h
#define wiring_h



// Stub out and emulate Arduino calls

#define PI 3.1415927

typedef uint16_t word;
typedef uint8_t  byte;

typedef const char __FlashStringHelper;
typedef const char * PGM_P;

struct {
	void print(const char *cp) {
		printf("%s",cp);
	}

	void print(char c) {
		printf("%c",c);
	}

	void print(int i) {
		printf("%d",i);
	}

	void println(int i) {
		printf("%d\n",i);
	}

	void println(const char *cp) {
		printf("%s\n",cp);
	}
} Serial;

struct {
	void write(int n, byte b) { }
	byte read(int n) { return 0; }
} EEPROM;

struct EthernetClient {
	FILE *fp = 0;
	
	bool connected() { return true; }
	bool available() { return true; }
	byte read() { return 0; }
	void stop() { }
	void write(char *b, int n) {
		fwrite(b, n, 1, fp);
	}
	operator bool() { return true; }
} client;

struct {
	EthernetClient available() { return client; }
} server;

unsigned long millis() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (ts.tv_sec/1000)*1000 + ts.tv_nsec/1000000;
}

unsigned long micros() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (ts.tv_sec/1000000)*1000000 + ts.tv_nsec/1000;
}

void delay(int ms) {
	usleep(ms*1000);
}

void interrupts() {}
void noInterrupts() {}
void attachInterrupt(int irq, void (*func)(), int type) {}
bool digitalRead(int pin) { return false; }
void digitalWrite(int pin, bool state) {}
void pinMode(int pin, int mode) {}
int digitalPinToInterrupt(int pin) { return pin; }
int analogRead(int pin) { return 0; }
#define HIGH true
#define LOW false
#define INPUT_PULLUP 0
#define OUTPUT 0
#define INPUT 0
#define RISING 0
#define FALLING 0



#endif /* wiring_h */

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

#ifndef i2cLED_h
#define i2cLED_h



void ledHoriz(int on, int decimalPt) {
	if (on)
		printf("#########");
	else
		printf("         ");
	if (decimalPt)
		printf(" #   ");
	else
		printf("     ");
}

void ledVert(int left, int right, int colon) {
	if (left)
		printf("#       ");
	else
		printf("        ");
	if (right)
		printf("#");
	else
		printf(" ");
	if (colon)
		printf("  #  ");
	else
		printf("     ");
}

byte i2cAddress;
byte *i2cPtr;
byte i2cBuffer[20];
bool i2cBlink;


void ledAllHoriz(int on, int decimalPt) {
	ledHoriz(i2cBuffer[1] & on, i2cBuffer[1] & decimalPt);
	ledHoriz(i2cBuffer[3] & on, i2cBuffer[3] & decimalPt);
	ledHoriz(i2cBuffer[7] & on, i2cBuffer[7] & decimalPt);
	ledHoriz(i2cBuffer[9] & on, i2cBuffer[9] & decimalPt);
	printf("\n");
}

void ledAllVert(int left, int right, bool colon) {
	ledVert(i2cBuffer[1] & left, i2cBuffer[1] & right, false);
	ledVert(i2cBuffer[3] & left, i2cBuffer[3] & right, colon ? (i2cBuffer[5] & 0x2) : 0);
	ledVert(i2cBuffer[7] & left, i2cBuffer[7] & right, false);
	ledVert(i2cBuffer[9] & left, i2cBuffer[9] & right, false);
	printf("\n");
}

#define I2C_WRITE 0
void i2c_init() {}
bool i2c_start(byte address) {
	i2cAddress = address;
	i2cPtr = i2cBuffer;
	return true;
}
bool i2c_write(byte b) {
	*i2cPtr++ = b;
	return true;
}
void i2c_stop() {
	// E0 - top display
	// E2 - botttom display
	if (i2cBuffer[0] == 0) {
		ledAllHoriz(1, 0);
		ledAllVert(0x20,2,false);
		ledAllVert(0x20,2,true);
		ledAllVert(0x20,2,false);
		ledAllHoriz(0x40, 0);
		ledAllVert(0x10,4,false);
		ledAllVert(0x10,4,true);
		ledAllVert(0x10,4,false);
		ledAllHoriz(8, 0x80);
		if (i2cBlink)
			printf("(blinking) ");
		else
			printf("           ");
		if (i2cAddress == 0xE0) {
			if (i2cBuffer[2] == 0x1)
				printf("Green \n\n");
			else if (i2cBuffer[2] == 0x5)
				printf("Yellow\n\n");
			else if (i2cBuffer[2] == 0x4)
				printf("Red   \n\n");
			else
				printf("Off   \n\n");
		}
	}
	if (i2cBuffer[0] & 0x80)
		i2cBlink = ((i2cBuffer[0] & 0xC) != 0);
}

#define F(x)	x
#define memcpy_P	memcpy
#define strlen_P	strlen



#endif /* i2cLED_h */

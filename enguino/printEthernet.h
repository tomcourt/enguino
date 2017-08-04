// Copyright 2017, Thomas Court
//
// Performance 'print' functions to ethernet
// -----------------------------------------
// Outputs to 'client', remember to 'flush()' the buffer after last print

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





char buffer[64];
byte bfree = 64;
char *bufp = buffer;

void flush() {
  client.write(buffer, 64-bfree);
  bfree = 64;
  bufp = buffer;
}

void print(string cp, short len) {
  while (len) {
    short n = len;
    if (n > bfree)
      n = bfree;
    memcpy(bufp, cp, n);
    bufp += n;
    bfree -= n;
    if (bfree == 0)
      flush();
    cp += n;
    len -= n;
  }
}

void print_P(const __FlashStringHelper * cp, short len) {
  PGM_P pp = reinterpret_cast<PGM_P>(cp);
  while (len) {
    short n = len;
    if (n > bfree)
      n = bfree;
    memcpy_P(bufp, pp, n);
    bufp += n;
    bfree -= n;
    if (bfree == 0)
      flush();
    pp += n;
    len -= n;
  }
}

void print(string cp) {
  print(cp, strlen(cp));
}

void print_P(const __FlashStringHelper * cp) {
  PGM_P pp = reinterpret_cast<PGM_P>(cp);
  print_P(cp, strlen_P(pp));
}

void print(char c) {
  print(&c,1);
}

void print(int n, short decimal) {
  char buf[7];
  buf[6] = 0;
  char *cp = buf + 6;
  if (n < 0) {
    if (n == FAULT) {
      print("inop");
      return;
    }

    print('-');
    n = -n;
  }
  do {
    *--cp = n % 10 + '0';
    n /= 10;
    if (decimal-- == 1)
      *--cp = '.';
  } while (n > 0 || decimal >= 0);
  print(cp);
}

void print(int n) {
  print(n,0);
}

void print_n_close() {
  print_P(F("'/>\n"));
}

void print_n_close(int n) {
  print(n);
  print_n_close();
}

void print_g_translate(short x, short y) {
  print_P(F("<g transform='translate("));
  print(x);
  print(' ');
  print(y);
  print_P(F(")'>\n"));
}

void print_g_close() {
  print_P(F("</g>\n"));
}

void print_text_close() {
  print_P(F("</text>\n"));
}

void print_text_close(string cp) {
  print(cp);
  print_text_close();
}

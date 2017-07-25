// Update the first 3 numbers of this IP address to your local network
// for testing but restore it to (192, 168, 0, 111) when finished.
IPAddress ip(192, 168, 0, 111);

// A made up MAC address. Only real critera is the first bytes 2 lsb must be 1 (for local) and 0 (for unicast).
byte mac[] = {  0xDE, 0x15, 0x24, 0x33, 0x42, 0x51 };

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP)

EthernetServer server(80);
EthernetClient client;



// Performance 'print' functions to ethernet 'client' using buffer (includes flush function)

char buffer[64];
int bfree = 64;
char *bufp = buffer;

void flush() {
  client.write(buffer, 64-bfree);
  bfree = 64;
  bufp = buffer;
}

void print(string cp, int len) {
  while (len) {
    int n = len;
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

void print_P(const __FlashStringHelper * cp, int len) {
  PGM_P pp = reinterpret_cast<PGM_P>(cp);
  while (len) {
    int n = len;
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

void print(int n, int decimal) {
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

void print_g_translate(int x, int y) {
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


#include "seg7.h"
#include <stdio.h>
#include <string.h>
#ifdef ARDUINO
#include <Arduino.h>
#endif

uint8_t segmentsFor(char c) {
  //                                    0     1     2     3     4     5     6     7     8     9
  static const uint8_t DIGIT[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
  if (c >= '0' && c <= '9') return DIGIT[c - '0'];
  switch (c) {
    case 'A': case 'a': return 0x77;
    case 'b': case 'B': return 0x7C;
    case 'C': return 0x39;
    case 'c': return 0x58;
    case 'd': case 'D': return 0x5E;
    case 'E': case 'e': return 0x79;
    case 'F': case 'f': return 0x71;
    case 'G': case 'g': return 0x3D;
    case 'H': return 0x76;
    case 'h': return 0x74;
    case 'I': case 'i': return 0x06;
    case 'J': case 'j': return 0x1E;
    case 'L': case 'l': return 0x38;
    case 'n': case 'N': return 0x54;
    case 'O': return 0x3F;
    case 'o': return 0x5C;
    case 'P': case 'p': return 0x73;
    case 'q': case 'Q': return 0x67;
    case 'r': case 'R': return 0x50;
    case 'S': case 's': return 0x6D;
    case 't': case 'T': return 0x78;
    case 'U': return 0x3E;
    case 'u': return 0x1C;
    case 'y': case 'Y': return 0x6E;
    case '-': return 0x40;
    case '_': return 0x08;
    default: return 0;
  }
}

#ifdef ARDUINO
void Seg7::begin(const uint8_t segPins[8], Kind k) {
  kind = k;
  for (uint8_t i = 0; i < 8; i++) { seg[i] = segPins[i]; pinMode(seg[i], OUTPUT); }
  off();
}

void Seg7::raw(uint8_t bits) {
  for (uint8_t i = 0; i < 8; i++) digitalWrite(seg[i], ((bits >> i) & 1) != (kind == ANODE));
}

void Seg7x4::begin(const uint8_t segPins[8], const uint8_t digitPins[4], Seg7::Kind k) {
  kind = k;
  for (uint8_t i = 0; i < 8; i++) { seg[i] = segPins[i]; pinMode(seg[i], OUTPUT); }
  for (uint8_t i = 0; i < 4; i++) {
    dig[i] = digitPins[i];
    pinMode(dig[i], OUTPUT);
    digitalWrite(dig[i], kind == Seg7::CATHODE ? HIGH : LOW);   // all digits off
  }
  segs(0);
}

void Seg7x4::segs(uint8_t bits) {
  for (uint8_t i = 0; i < 8; i++) digitalWrite(seg[i], ((bits >> i) & 1) != (kind == Seg7::ANODE));
}

void Seg7x4::update() {
  uint32_t now = micros();
  if (now - lastUs < digitUs) return;
  lastUs = now;
  bool cathode = kind == Seg7::CATHODE;
  digitalWrite(dig[current], cathode ? HIGH : LOW);   // previous digit off first: no ghosting
  current = (current + 1) & 3;
  segs(buf[current]);
  digitalWrite(dig[current], cathode ? LOW : HIGH);
}
#endif

void Seg7x4::number(int16_t n, uint8_t dotAt) {
  clear();
  if (n > 9999 || n < -999) { for (uint8_t i = 0; i < 4; i++) buf[i] = 0x40; return; }
  bool neg = n < 0;
  uint16_t v = neg ? -n : n;
  int8_t i = 3;
  do { buf[i--] = segmentsFor('0' + v % 10); v /= 10; } while (v && i >= 0);
  if (neg && i >= 0) buf[i] = 0x40;
  if (dotAt < 4) buf[dotAt] |= 0x80;
}

void Seg7x4::fixed(int16_t value, uint8_t decimals) {
  if (decimals > 3) decimals = 3;
  uint16_t scale = decimals == 0 ? 1 : decimals == 1 ? 10 : decimals == 2 ? 100 : 1000;
  bool neg = value < 0;
  uint16_t a = neg ? -(int32_t)value : value;
  char s[12];
  if (decimals) snprintf(s, sizeof(s), "%s%u.%0*u", neg ? "-" : "", a / scale, decimals, a % scale);
  else snprintf(s, sizeof(s), "%s%u", neg ? "-" : "", a);
  uint8_t visible = 0;
  for (const char *p = s; *p; p++) if (*p != '.') visible++;
  if (visible > 4) { for (uint8_t i = 0; i < 4; i++) buf[i] = 0x40; return; }   // doesn't fit: "----"
  char padded[16] = "    ";
  padded[4 - visible] = 0;
  strcat(padded, s);
  text(padded);
}

void Seg7x4::text(const char *s) {
  clear();
  for (uint8_t i = 0; i < 4 && *s; s++) {
    if (*s == '.' && i > 0) { buf[i - 1] |= 0x80; continue; }
    buf[i++] = segmentsFor(*s);
    if (s[1] == '.') { buf[i - 1] |= 0x80; s++; }
  }
}

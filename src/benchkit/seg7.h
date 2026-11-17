// 7-segment displays. Segment pins a b c d e f g dp, each through its own 220-330 Ω resistor.
//
//   const uint8_t SEGS[8] = {2, 3, 4, 5, 6, 7, 8, 9};      // a b c d e f g dp
//
//   Seg7 one;    one.begin(SEGS);  one.show('7');  one.show(3, true);    // digit 3 with its dot
//
//   const uint8_t DIGITS[4] = {10, 11, 12, 13};             // digit 1-4 (common pins), left to right
//   Seg7x4 four; four.begin(SEGS, DIGITS);
//   four.number(1234);  four.text("HELP");  four.update();  // update() every loop: lights one digit at a time
//
// 5161AS / 5461AS are common cathode (the digit pins go to GND through the board): the default.
// Common anode: begin(SEGS, Seg7::ANODE). Four digits share the segment pins and take turns
// ~2 ms each, so update() must run often; a 4-digit display drawing ~20 mA per segment is
// fine from pins only because one digit is lit at a time.
// Through a 74HC595 instead: segmentsFor('7') gives the byte (bit 0 = a .. bit 7 = dp).
#pragma once
#include <stdint.h>

// Segment byte for a character: bit 0 = a, 1 = b ... 6 = g, 7 = dp. Digits 0-9, letters
// that work on 7 segments (A b C c d E F G H h I J L n O o P q r S t U u y), '-', '_', ' '.
// Anything else = blank.
uint8_t segmentsFor(char c);

class Seg7 {
public:
  enum Kind : uint8_t { CATHODE, ANODE };
  void begin(const uint8_t segPins[8], Kind kind = CATHODE);
  void show(char c, bool dot = false) { raw(segmentsFor(c) | (dot ? 0x80 : 0)); }
  void show(uint8_t digit, bool dot) { show((char)('0' + digit % 10), dot); }
  void raw(uint8_t segments);    // your own pattern
  void off() { raw(0); }
private:
  uint8_t seg[8];
  Kind kind = CATHODE;
};

class Seg7x4 {
public:
  uint16_t digitUs = 2000;       // how long each digit stays lit per turn

  void begin(const uint8_t segPins[8], const uint8_t digitPins[4], Seg7::Kind kind = Seg7::CATHODE);
  // -999..9999, right-aligned; out of range shows "----". dotAt: digit 0-3 that gets a dot (255 = none)
  void number(int16_t n, uint8_t dotAt = 255);
  // A number with a decimal point: fixed(215, 1) -> " 21.5"
  void fixed(int16_t tenthsOrHundredths, uint8_t decimals);
  void text(const char *s);      // up to 4 characters; '.' after a character lights its dot
  void raw(uint8_t digit, uint8_t segments) { if (digit < 4) buf[digit] = segments; }
  void clear() { for (uint8_t i = 0; i < 4; i++) buf[i] = 0; }
  void update();                 // every loop
  uint8_t at(uint8_t digit) const { return buf[digit & 3]; }

private:
  uint8_t seg[8], dig[4], buf[4] = {};
  Seg7::Kind kind = Seg7::CATHODE;
  uint8_t current = 0;
  uint32_t lastUs = 0;
  void segs(uint8_t bits);
};

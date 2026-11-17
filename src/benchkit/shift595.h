// 74HC595 shift registers: 8 outputs per chip from 3 pins, chips chained for more.
//
//   Shift595<1> sr;                       // <2> for two chained chips (16 outputs), and so on
//   sr.begin(DATA_PIN, CLOCK_PIN, LATCH_PIN);
//   sr.set(3, true);  sr.setByte(0, 0b10100000);
//   sr.write();                           // sends, only if something changed since last time
//
// Chip pins: 14 DS = data, 11 SH_CP = clock, 12 ST_CP = latch, 16 VCC and 10 MR to 5V,
// 8 GND and 13 OE to GND, outputs Q0-Q7 = pins 15, 1-7. Chain: Q7' (pin 9) of chip 0 to DS of
// chip 1, clock and latch shared. A 100 nF capacitor across 5V/GND at each chip.
// Output n: chip n / 8, Q(n % 8). Each output can source ~6 mA comfortably: LEDs with 330 Ω+.
#pragma once
#include <stdint.h>
#include <string.h>

void shift595Send(uint8_t data, uint8_t clock, uint8_t latch, const uint8_t *bytes, uint8_t n);

template <uint8_t CHIPS = 1>
class Shift595 {
public:
  static const uint8_t OUTPUTS = CHIPS * 8;

  void begin(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);
  void set(uint8_t out, bool on) {
    if (out >= OUTPUTS) return;
    uint8_t &b = bits[out >> 3], m = 1 << (out & 7);
    b = on ? b | m : b & ~m;
  }
  bool get(uint8_t out) const { return out < OUTPUTS && (bits[out >> 3] >> (out & 7)) & 1; }
  void setByte(uint8_t chip, uint8_t value) { if (chip < CHIPS) bits[chip] = value; }
  uint8_t getByte(uint8_t chip) const { return chip < CHIPS ? bits[chip] : 0; }
  void clear() { memset(bits, 0, sizeof(bits)); }
  // Send if anything changed; true if it sent
  bool write() {
    if (!memcmp(bits, sent, sizeof(bits))) return false;
    writeNow();
    return true;
  }
  void writeNow() {
    shift595Send(pins[0], pins[1], pins[2], bits, CHIPS);
    memcpy(sent, bits, sizeof(bits));
  }

private:
  uint8_t pins[3] = {255, 255, 255};
  uint8_t bits[CHIPS] = {}, sent[CHIPS] = {};
};

template <uint8_t CHIPS>
void Shift595<CHIPS>::begin(uint8_t d, uint8_t c, uint8_t l) {
  pins[0] = d; pins[1] = c; pins[2] = l;
  clear();
  writeNow();   // all outputs off, whatever they powered up as
}

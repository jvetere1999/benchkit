#include "ssd1306.h"
#include <Arduino.h>
#include <Wire.h>

namespace ssd1306 {

bool present(uint8_t addr) {
  static bool started = false;
  if (!started) { Wire.begin(); Wire.setClock(400000); started = true; }
  if (!addr) return false;
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

void reset(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  delay(1);
  digitalWrite(pin, LOW);
  delay(10);
  digitalWrite(pin, HIGH);
  delay(1);
}

void command(uint8_t addr, uint8_t c) {
  Wire.beginTransmission(addr);
  Wire.write((uint8_t)0x00);   // Co = 0, D/C = 0: a command
  Wire.write(c);
  Wire.endTransmission();
}

void commandsNow(uint8_t addr, const uint8_t *ram, uint8_t n) {
  Wire.beginTransmission(addr);
  Wire.write((uint8_t)0x00);
  for (uint8_t i = 0; i < n; i++) Wire.write(ram[i]);
  Wire.endTransmission();
}

void commands(uint8_t addr, const uint8_t *list, uint8_t n) {
  for (uint8_t i = 0; i < n; i++) command(addr, pgm_read_byte(list + i));
}

void data(uint8_t addr, const uint8_t *bytes, uint8_t n) {
  Wire.beginTransmission(addr);
  Wire.write((uint8_t)0x40);   // D/C = 1: display data
  for (uint8_t i = 0; i < n; i++) Wire.write(bytes[i]);
  Wire.endTransmission();
}

// The start-up sequence (as Adafruit's library sends it, internal charge pump)
static const uint8_t INIT[] PROGMEM = {
  0xAE,         // display off
  0xD5, 0x80,   // clock
  0x8D, 0x14,   // charge pump on
  0x20, 0x00,   // horizontal addressing
  0xA1,         // columns left to right
  0xC8,         // rows top to bottom
  0xD9, 0xF1,   // pre-charge
  0xDB, 0x40,   // VCOMH
  0xA4,         // show the RAM
  0xA6,         // not inverted
  0x2E,         // no scrolling
  0xD3, 0x00,   // no offset
  0x40,         // start line 0
};

void init(uint8_t addr, uint8_t height) {
  commands(addr, INIT, sizeof(INIT));
  command(addr, 0xA8); command(addr, height - 1);                  // multiplex = rows
  command(addr, 0xDA); command(addr, height == 64 ? 0x12 : 0x02);  // COM pins
  command(addr, 0x81); command(addr, height == 64 ? 0xCF : 0x8F);  // contrast
  command(addr, 0xAF);                                             // display on
}

}

#include "lcd1602.h"
#include <Arduino.h>

void Lcd1602::nibble(uint8_t n) {
  for (uint8_t i = 0; i < 4; i++) digitalWrite(d[i], (n >> i) & 1);
  digitalWrite(en, HIGH);   // the LCD reads the pins as EN falls
  delayMicroseconds(1);
  digitalWrite(en, LOW);
  delayMicroseconds(40);    // most commands take 37 µs
}

void Lcd1602::write(uint8_t value, bool isData) {
  digitalWrite(rs, isData);
  nibble(value >> 4);
  nibble(value & 0x0F);
}

void Lcd1602::begin(uint8_t rsPin, uint8_t enPin, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
  rs = rsPin; en = enPin; d[0] = d4; d[1] = d5; d[2] = d6; d[3] = d7;
  pinMode(rs, OUTPUT);
  pinMode(en, OUTPUT);
  for (uint8_t i = 0; i < 4; i++) pinMode(d[i], OUTPUT);
  digitalWrite(rs, LOW);
  digitalWrite(en, LOW);
  delay(50);                       // power-up
  // Into 4-bit mode, whatever state it was in (the datasheet's "initialise by instruction")
  nibble(0x03); delayMicroseconds(4500);
  nibble(0x03); delayMicroseconds(4500);
  nibble(0x03); delayMicroseconds(150);
  nibble(0x02);
  write(0x28, false);              // 4-bit, 2 lines, 5x8 font
  write(0x0C, false);              // display on, no cursor, no blink
  write(0x06, false);              // move right after each character
  write(0x01, false);              // clear
  delayMicroseconds(2000);
  for (uint8_t r = 0; r < ROWS; r++) for (uint8_t c = 0; c < COLS; c++) shown[r][c] = ' ';
  cursor = 0;
}

void Lcd1602::put(uint8_t col, uint8_t row, char ch) {
  if (col >= COLS || row >= ROWS || shown[row][col] == ch) return;
  uint8_t pos = row * 64 + col;
  if (pos != cursor) write(0x80 | pos, false);   // move only when not already there
  write(ch, true);
  shown[row][col] = ch;
  cursor = pos + 1;
}

uint8_t Lcd1602::textImpl(uint8_t col, uint8_t row, const char *s, bool flash) {
  for (;; s++, col++) {
    char ch = flash ? pgm_read_byte(s) : *s;
    if (!ch || col >= COLS) break;
    put(col, row, ch);
  }
  return col;
}
uint8_t Lcd1602::text(uint8_t col, uint8_t row, const char *s) { return textImpl(col, row, s, false); }
uint8_t Lcd1602::text(uint8_t col, uint8_t row, const __FlashStringHelper *s) { return textImpl(col, row, (const char *)s, true); }

uint8_t Lcd1602::number(uint8_t col, uint8_t row, long value, uint8_t width) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%*ld", width, value);
  return text(col, row, buf);
}

void Lcd1602::customChar(uint8_t slot, const uint8_t *rows) {
  write(0x40 | ((slot & 7) << 3), false);
  for (uint8_t i = 0; i < 8; i++) write(pgm_read_byte(rows + i), true);
  cursor = 255;   // the next put() sets the position again (characters already showing that
}                 // slot change on their own)

void Lcd1602::display(bool on) { write(on ? 0x0C : 0x08, false); }

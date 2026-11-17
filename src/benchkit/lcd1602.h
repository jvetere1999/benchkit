// A 16x2 character LCD (HD44780, the 16-pin header kind) in 4-bit mode, on any 6 pins:
//
//   Lcd1602 lcd;
//   void setup() { lcd.begin(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7); }
//   void loop()  { lcd.text(0, 0, F("Temp")); lcd.number(6, 0, tempC, 3); }
//
// Wiring: VSS GND, VDD 5V, V0 to a 10 kΩ pot's middle (contrast; ends to 5V and GND), RW to GND,
// RS / E / D4-D7 to the pins you pass in, A (backlight +) through 220 Ω to 5V, K to GND.
// Blank screen with one row of boxes = contrast too high or begin() never ran.
//
// It keeps a copy of what's on the screen and only sends characters that changed, so writing
// the same text every loop costs nothing and nothing flickers (no lcd.clear() needed).
#pragma once
#include <stdint.h>
class __FlashStringHelper;

class Lcd1602 {
public:
  static const uint8_t COLS = 16, ROWS = 2;

  void begin(uint8_t rs, uint8_t en, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

  // Text at (col, row); cut off at the end of the row. Returns the column after it.
  uint8_t text(uint8_t col, uint8_t row, const char *s);
  uint8_t text(uint8_t col, uint8_t row, const __FlashStringHelper *s);
  // A number right-aligned in `width` characters (padded with spaces), e.g. number(6, 0, 42, 4) -> "  42"
  uint8_t number(uint8_t col, uint8_t row, long value, uint8_t width = 0);
  void put(uint8_t col, uint8_t row, char c);
  void clearRow(uint8_t row, uint8_t from = 0) { for (uint8_t c = from; c < COLS; c++) put(c, row, ' '); }
  void clear() { for (uint8_t r = 0; r < ROWS; r++) clearRow(r); }

  // Your own character: slot 0-7, 8 rows of 5 bits (PROGMEM). Show it with put(col, row, slot).
  void customChar(uint8_t slot, const uint8_t *rows);
  void display(bool on);   // turn the characters off / on (the backlight is wired separately)

  char at(uint8_t col, uint8_t row) const { return shown[row][col]; }

private:
  uint8_t rs = 255, en, d[4];
  char shown[ROWS][COLS];
  uint8_t cursor = 255;   // where the LCD will write next (row * 64 + col), 255 = unknown
  void write(uint8_t value, bool isData);
  void nibble(uint8_t n);
  uint8_t textImpl(uint8_t col, uint8_t row, const char *s, bool flash);
};

// A log that prints to serial (or any Print) and also keeps its last few lines, for a screen
// to show:
//
//   ScreenLog<26, 9> Log(Serial);     // 26 characters x 9 rows (160 px wide screen, 6 px font)
//   stamp(Log); Log.println(F("Button tapped"));   // goes to serial, and into the rows
//   if (Log.version() != drawn) {                  // a row changed: redraw them
//     for (uint8_t r = 0; r < Log.rows; r++) tft.text(0, 60 + r * 8, Log.row(r), WHITE, BLACK);
//     drawn = Log.version();
//   }
//
// Row 0 is the oldest, the last row the newest. Every row is `cols` characters, padded with
// spaces (so drawing it overwrites the old text). Long lines wrap onto the next row.
// LogRows is the part without serial (tested on the computer).
#pragma once
#include <stdint.h>
#include <string.h>

template <uint8_t COLS, uint8_t ROWS>
class LogRows {
public:
  static const uint8_t cols = COLS, rows = ROWS;

  LogRows() {
    for (uint8_t r = 0; r < ROWS; r++) { memset(text[r], ' ', COLS); text[r][COLS] = 0; }
  }

  void add(char c) {
    if (c == '\r') return;
    if (c == '\n') { newline = true; return; }
    if (newline || col == COLS) {   // start the next row (the oldest one is dropped)
      head = (head + 1) % ROWS;
      memset(text[head], ' ', COLS);
      col = 0;
      newline = false;
    }
    text[head][col++] = c;
    changes++;
  }

  const char *row(uint8_t r) const { return text[(head + 1 + r) % ROWS]; }   // 0 = oldest
  uint16_t version() const { return changes; }   // goes up whenever a row changes

private:
  char text[ROWS][COLS + 1];
  uint8_t head = 0, col = 0;   // `head` = the newest row
  bool newline = true;         // start a fresh row on the next character
  uint16_t changes = 0;
};

#if defined(ARDUINO) && !defined(NATIVE_TEST)
#include <Arduino.h>

template <uint8_t COLS, uint8_t ROWS>
class ScreenLog : public Print, public LogRows<COLS, ROWS> {
public:
  explicit ScreenLog(Print &out) : out(out) {}
  size_t write(uint8_t c) override {
    out.write(c);
    this->add((char)c);
    return 1;
  }
  using Print::write;

private:
  Print &out;
};
#endif

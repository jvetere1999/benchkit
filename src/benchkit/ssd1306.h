// An SSD1306 monochrome OLED over I2C (SDA/SCL: A4/A5 on an Uno, 20/21 on a Mega):
//
//   Ssd1306 oled;                        // 128 x 64 (1 KB of RAM). Ssd1306x32 for 128 x 32 (512 B)
//   void setup() { oled.begin(); }       // finds it at 0x3C or 0x3D; false if nothing answers
//   void loop()  { oled.text(0, 0, F("Hi"), OLED_ON, OLED_OFF); oled.show(); }
//
// Drawing (text, lines, circles, bars, sprites) comes from gfx.h and only changes the buffer in
// RAM. show() sends just the columns that changed since the last show(), per 8-pixel row, so a
// small change costs a few bytes on the wire instead of the whole 1 KB screen.
// Colours: OLED_ON (lit) / OLED_OFF; anything non-zero counts as lit.
#pragma once
#include <stdint.h>
#include <string.h>
#include "gfx.h"

#define OLED_OFF 0
#define OLED_ON  1

// The I2C part, not a template (one copy for every size)
namespace ssd1306 {
bool present(uint8_t addr);
void commands(uint8_t addr, const uint8_t *list, uint8_t n);   // list in flash
void command(uint8_t addr, uint8_t c);
void commandsNow(uint8_t addr, const uint8_t *ram, uint8_t n);   // several, one transmission
void data(uint8_t addr, const uint8_t *bytes, uint8_t n);
void init(uint8_t addr, uint8_t height);
}

template <uint8_t H>
class Ssd1306T : public Gfx<Ssd1306T<H>> {
public:
  static const uint8_t W = 128, PAGES = H / 8;
  Ssd1306T() { for (uint8_t p = 0; p < PAGES; p++) { lo[p] = 255; hi[p] = 0; } }

  // address 0 = try 0x3C, then 0x3D. Returns false if no OLED answers (check SDA/SCL/power).
  bool begin(uint8_t address = 0) {
    addr = address ? address : ssd1306::present(0x3C) ? 0x3C : 0x3D;
    if (!ssd1306::present(addr)) return false;
    ssd1306::init(addr, H);
    clear();
    show();   // clear() marked everything: the whole screen goes out once
    return true;
  }
  uint8_t address() const { return addr; }

  int16_t width() const { return W; }
  int16_t height() const { return H; }

  void clear() { memset(buf, 0, sizeof(buf)); markAll(); }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > W) w = W - x;
    if (y + h > H) h = H - y;
    if (w <= 0 || h <= 0) return;
    for (int16_t yy = y; yy < y + h;) {   // one page (8 rows) at a time
      uint8_t page = yy >> 3, top = yy & 7;
      uint8_t rows = 8 - top;
      if (rows > y + h - yy) rows = y + h - yy;
      uint8_t mask = (uint8_t)(((1u << rows) - 1) << top);
      uint8_t *col = &buf[page * W + x];
      bool changed = false;
      for (int16_t i = 0; i < w; i++) {
        uint8_t v = c ? (col[i] | mask) : (col[i] & ~mask);
        if (v != col[i]) { col[i] = v; changed = true; }
      }
      if (changed) mark(page, x, x + w - 1);
      yy += rows;
    }
  }

  bool get(int16_t x, int16_t y) const {
    return x >= 0 && y >= 0 && x < W && y < H && (buf[(y >> 3) * W + x] >> (y & 7)) & 1;
  }

  // Send what changed. Call once per frame, after drawing.
  void show() {
    for (uint8_t p = 0; p < PAGES; p++) {
      if (lo[p] > hi[p]) continue;
      const uint8_t window[] = {0x21, lo[p], hi[p], 0x22, p, p};   // columns, page
      ssd1306::commandsNow(addr, window, sizeof(window));
      for (uint8_t c = lo[p]; c <= hi[p];) {   // 16 bytes per transmission (fits every Wire buffer)
        uint8_t n = hi[p] - c + 1;
        if (n > 16) n = 16;
        ssd1306::data(addr, &buf[p * W + c], n);
        c += n;
      }
      lo[p] = 255; hi[p] = 0;
    }
  }

  void contrast(uint8_t level) { ssd1306::command(addr, 0x81); ssd1306::command(addr, level); }
  void invert(bool on) { ssd1306::command(addr, on ? 0xA7 : 0xA6); }
  void power(bool on) { ssd1306::command(addr, on ? 0xAF : 0xAE); }
  uint8_t *buffer() { return buf; }   // raw access: page-major, bit 0 = top row of the page
  void markAll() { for (uint8_t p = 0; p < PAGES; p++) { lo[p] = 0; hi[p] = W - 1; } }

private:
  uint8_t buf[W * PAGES];
  uint8_t lo[PAGES], hi[PAGES];   // changed columns per page since the last show(); lo > hi = none
  uint8_t addr = 0x3C;
  void mark(uint8_t p, uint8_t a, uint8_t b) { if (a < lo[p]) lo[p] = a; if (b > hi[p]) hi[p] = b; }
};

typedef Ssd1306T<64> Ssd1306;
typedef Ssd1306T<32> Ssd1306x32;

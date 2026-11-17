// A 1.8" ST7735 colour TFT (160 x 128) on software SPI, on any pins (passed in at begin()):
//
//   St7735 tft;
//   void setup() {
//     tft.begin(TFT_MOSI, TFT_SCLK, TFT_DC, TFT_CS, TFT_RST);   // landscape 160 x 128, cleared
//     tft.text(4, 4, F("Hello"), WHITE, BLACK);
//   }
//
// Drawing (lines, circles, bars, text, sprites) comes from gfx.h; this class makes the
// rectangles, text and sprites fast. Why not just Adafruit's library: its software SPI sends
// each bit through general-purpose code (~40 µs a byte) and sets a window per pixel. Here the
// pins' port registers are looked up once at begin(), bits are flipped through them (roughly
// 10 µs a byte on an Uno, estimated from the code; the extras/dev demo shows real frame times), and
// each rectangle, character or sprite is one window. Adafruit's object still sends the start-up
// commands, and is there for anything rare: tft.lib().
#pragma once
#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include "gfx.h"

#define BLACK   ST77XX_BLACK
#define WHITE   ST77XX_WHITE

class St7735 : public Gfx<St7735> {
public:
  // tab: INITR_BLACKTAB for most red 1.8" boards; try INITR_GREENTAB / INITR_REDTAB if an edge
  // shows a stripe of noise or the colours are swapped. rotation 1/3 = landscape 160 x 128,
  // 0/2 = portrait 128 x 160.
  void begin(uint8_t mosi, uint8_t sck, uint8_t dc, uint8_t cs, int8_t rst = -1,
             uint8_t tab = INITR_BLACKTAB, uint8_t rotation = 1);

  int16_t width() const { return w; }
  int16_t height() const { return h; }
  Adafruit_ST7735 &lib();   // Adafruit's object (after begin), for anything not here

  void fillRect(int16_t x, int16_t y, int16_t rw, int16_t rh, uint16_t c);
  int16_t text(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale = 1) {
    return fastText(x, y, s, false, fg, bg, scale);
  }
  int16_t text(int16_t x, int16_t y, const __FlashStringHelper *s, uint16_t fg, uint16_t bg, uint8_t scale = 1) {
    return fastText(x, y, (const char *)s, true, fg, bg, scale);
  }
  using Gfx<St7735>::sprite;
  void sprite(int16_t wx, int16_t wy, int16_t ww, int16_t wh, int16_t sx, int16_t sy,
              const uint16_t *rows, uint8_t sw, uint8_t sh, uint16_t fg, uint16_t bg);

  // Low level, for your own fast drawing: select the screen and set the rectangle the
  // following pixels fill (row by row), send them, then end().
  void window(int16_t x, int16_t y, int16_t ww, int16_t wh);
  void send(uint16_t c);
  void sendRepeat(uint16_t c, uint32_t n);
  void end();

private:
  int16_t w = 160, h = 128, x0 = 0, y0 = 0;   // size, and the panel's column/row offset
#ifdef __AVR__
  volatile uint8_t *mosiPort, *sckPort, *dcPort, *csPort;
  uint8_t mosiMask, sckMask, dcMask, csMask;
#else
  uint8_t mosiPin, sckPin, dcPin, csPin;
#endif
  void byte(uint8_t b);
  void command(uint8_t c);
  bool clip(int16_t &x, int16_t &y, int16_t &rw, int16_t &rh) const;
  int16_t fastText(int16_t x, int16_t y, const char *s, bool flash, uint16_t fg, uint16_t bg, uint8_t scale);
};

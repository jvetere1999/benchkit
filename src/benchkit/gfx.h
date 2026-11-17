// Drawing shared by the graphic displays. A display class inherits Gfx<itself> and provides:
//
//   int16_t width() const, height() const
//   void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colour)   (clipped)
//
// and gets everything below. A display can also provide its own faster version of any of
// these (St7735 has its own text and sprite); calls on the display then use that one.
// It's a template, not virtual functions, so only what a sketch calls reaches the board.
//
// Colours are uint16_t: RGB565 on colour screens (color565()), 0 / 1 on the OLED.
#pragma once
#include <stdint.h>
#include "font5x7.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#define pgm_read_word(p) (*(const uint16_t *)(p))
#endif
class __FlashStringHelper;

// 16-bit colour from 0..255 red, green, blue
constexpr uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}
// Rainbow: 0..255 goes red -> yellow -> green -> cyan -> blue -> magenta -> red
uint16_t hue565(uint8_t h);
// A colour's brightness scaled by k: 0 = black .. 255 = unchanged
uint16_t dim565(uint16_t c, uint8_t k);

template <class D>
class Gfx {
public:
  void fill(uint16_t c) { self().fillRect(0, 0, self().width(), self().height(), c); }
  void pixel(int16_t x, int16_t y, uint16_t c) { self().fillRect(x, y, 1, 1, c); }
  void hLine(int16_t x, int16_t y, int16_t len, uint16_t c) { self().fillRect(x, y, len, 1, c); }
  void vLine(int16_t x, int16_t y, int16_t len, uint16_t c) { self().fillRect(x, y, 1, len, c); }

  void rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) {   // outline
    hLine(x, y, w, c);
    hLine(x, y + h - 1, w, c);
    vLine(x, y + 1, h - 2, c);
    vLine(x + w - 1, y + 1, h - 2, c);
  }

  void line(int16_t xa, int16_t ya, int16_t xb, int16_t yb, uint16_t c) {
    if (ya == yb) { if (xb < xa) swap(xa, xb); hLine(xa, ya, xb - xa + 1, c); return; }
    if (xa == xb) { if (yb < ya) swap(ya, yb); vLine(xa, ya, yb - ya + 1, c); return; }
    int16_t dx = xb > xa ? xb - xa : xa - xb, dy = yb > ya ? ya - yb : yb - ya;   // Bresenham
    int16_t sx = xa < xb ? 1 : -1, sy = ya < yb ? 1 : -1, err = dx + dy;
    for (;;) {
      self().pixel(xa, ya, c);
      if (xa == xb && ya == yb) break;
      int16_t e2 = 2 * err;
      if (e2 >= dy) { err += dy; xa += sx; }
      if (e2 <= dx) { err += dx; ya += sy; }
    }
  }

  void fillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t c) {
    int16_t dx = r;
    for (int16_t dy = 0; dy <= r; dy++) {   // one span per row, mirrored above and below
      while (dx > 0 && (int32_t)dx * dx + (int32_t)dy * dy > (int32_t)r * r) dx--;
      hLine(cx - dx, cy + dy, 2 * dx + 1, c);
      if (dy) hLine(cx - dx, cy - dy, 2 * dx + 1, c);
    }
  }

  void circle(int16_t cx, int16_t cy, int16_t r, uint16_t c) {   // outline
    int16_t x = r, y = 0, err = 1 - r;
    while (x >= y) {
      D &d = self();
      d.pixel(cx + x, cy + y, c); d.pixel(cx - x, cy + y, c); d.pixel(cx + x, cy - y, c); d.pixel(cx - x, cy - y, c);
      d.pixel(cx + y, cy + x, c); d.pixel(cx - y, cy + x, c); d.pixel(cx + y, cy - x, c); d.pixel(cx - y, cy - x, c);
      y++;
      if (err < 0) err += 2 * y + 1;
      else { x--; err += 2 * (y - x) + 1; }
    }
  }

  // A bar / gauge: value/max of the inside in fg, the rest in bg, with an outline
  void bar(int16_t x, int16_t y, int16_t w, int16_t h, int16_t value, int16_t max,
           uint16_t fg, uint16_t bg, uint16_t outline) {
    rect(x, y, w, h, outline);
    int16_t inner = w - 2;
    if (value < 0) value = 0;
    if (value > max) value = max;
    int16_t on = max > 0 ? (int32_t)inner * value / max : 0;
    self().fillRect(x + 1, y + 1, on, h - 2, fg);
    self().fillRect(x + 1 + on, y + 1, inner - on, h - 2, bg);
  }

  // Text in the 5x7 font on a 6x8 grid, background included (writes over old text).
  // scale 2 = 12x16. Returns the x after the last character drawn.
  int16_t text(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale = 1) {
    return textImpl(x, y, s, false, fg, bg, scale);
  }
  int16_t text(int16_t x, int16_t y, const __FlashStringHelper *s, uint16_t fg, uint16_t bg, uint8_t scale = 1) {
    return textImpl(x, y, (const char *)s, true, fg, bg, scale);
  }
  static int16_t textWidth(uint8_t chars, uint8_t scale = 1) { return chars * 6 * scale; }

  // A 1-bit sprite: rows in PROGMEM, one uint16_t per row, leftmost pixel = highest bit (width
  // up to 16), at (sx, sy), drawn through the window (wx, wy, ww, wh): window pixels the sprite
  // doesn't cover become bg. Cover last frame's position too and it's erased in the same go.
  void sprite(int16_t wx, int16_t wy, int16_t ww, int16_t wh, int16_t sx, int16_t sy,
              const uint16_t *rows, uint8_t sw, uint8_t sh, uint16_t fg, uint16_t bg) {
    for (int16_t py = wy; py < wy + wh; py++) {
      int16_t r = py - sy;
      uint16_t bits = (r >= 0 && r < sh) ? pgm_read_word(&rows[r]) : 0;
      for (int16_t px = wx; px < wx + ww; px++) {
        int16_t c = px - sx;
        self().pixel(px, py, c >= 0 && c < sw && (bits & (0x8000 >> c)) ? fg : bg);
      }
    }
  }
  void sprite(int16_t x, int16_t y, const uint16_t *rows, uint8_t sw, uint8_t sh, uint16_t fg, uint16_t bg) {
    self().sprite(x, y, sw, sh, x, y, rows, sw, sh, fg, bg);
  }

protected:
  D &self() { return *static_cast<D *>(this); }
  static void swap(int16_t &a, int16_t &b) { int16_t t = a; a = b; b = t; }

  int16_t textImpl(int16_t x, int16_t y, const char *s, bool flash, uint16_t fg, uint16_t bg, uint8_t scale) {
    if (!scale) scale = 1;
    for (;; s++, x += 6 * scale) {
      char ch = flash ? pgm_read_byte(s) : *s;
      if (!ch || x + 6 * scale > self().width()) break;
      for (uint8_t col = 0; col < 6; col++) {
        uint8_t bits = col < 5 ? pgm_read_byte(&FONT5X7[(uint8_t)ch * 5 + col]) : 0;
        for (uint8_t row = 0; row < 8; row++)   // one scale x scale block per font pixel
          self().fillRect(x + col * scale, y + row * scale, scale, scale, (bits >> row) & 1 ? fg : bg);
      }
    }
    return x;
  }
};

// Built only when Adafruit's ST7735 library is installed (it is a dependency, but a
// native build or a trimmed install may leave it out).
#if __has_include(<Adafruit_ST7735.h>)
#include "st7735.h"
#include <new>

// Adafruit's object, built at begin() once the pins are known (no heap: it lives in `store`).
// The subclass only exposes the panel's column/row offset, which differs between screen "tabs".
namespace {
class Lib : public Adafruit_ST7735 {
public:
  using Adafruit_ST7735::Adafruit_ST7735;
  int16_t xOffset() const { return _xstart; }
  int16_t yOffset() const { return _ystart; }
};
alignas(Lib) uint8_t store[sizeof(Lib)];
Lib *adafruit = nullptr;
}

Adafruit_ST7735 &St7735::lib() { return *adafruit; }

void St7735::begin(uint8_t mosi, uint8_t sck, uint8_t dc, uint8_t cs, int8_t rst, uint8_t tab, uint8_t rotation) {
  if (!adafruit) adafruit = new (store) Lib(cs, dc, mosi, sck, rst);   // software SPI
  adafruit->initR(tab);
  adafruit->setRotation(rotation);
  w = adafruit->width();
  h = adafruit->height();
  x0 = adafruit->xOffset();
  y0 = adafruit->yOffset();
#ifdef __AVR__
  mosiPort = portOutputRegister(digitalPinToPort(mosi)); mosiMask = digitalPinToBitMask(mosi);
  sckPort = portOutputRegister(digitalPinToPort(sck));   sckMask = digitalPinToBitMask(sck);
  dcPort = portOutputRegister(digitalPinToPort(dc));     dcMask = digitalPinToBitMask(dc);
  csPort = portOutputRegister(digitalPinToPort(cs));     csMask = digitalPinToBitMask(cs);
#else
  mosiPin = mosi; sckPin = sck; dcPin = dc; csPin = cs;
#endif
  fill(BLACK);
}

// --- Bits on the wire ---------------------------------------------------------------------

#ifdef __AVR__
// One byte, most significant bit first. Port and masks are copied into registers first, so
// each bit is a handful of instructions. (Read-modify-write on the port: don't drive other
// pins on these ports from an interrupt.)
void St7735::byte(uint8_t b) {
  volatile uint8_t *mp = mosiPort, *sp = sckPort;
  uint8_t mHi = mosiMask, mLo = ~mosiMask, sHi = sckMask, sLo = ~sckMask;
  for (uint8_t bit = 0x80; bit; bit >>= 1) {
    if (b & bit) *mp |= mHi; else *mp &= mLo;
    *sp |= sHi;
    *sp &= sLo;
  }
}
#define DC_CMD()   (*dcPort &= ~dcMask)
#define DC_DATA()  (*dcPort |= dcMask)
#define CS_ON()    (*csPort &= ~csMask)
#define CS_OFF()   (*csPort |= csMask)
#else
void St7735::byte(uint8_t b) {
  for (uint8_t bit = 0x80; bit; bit >>= 1) {
    digitalWrite(mosiPin, b & bit ? HIGH : LOW);
    digitalWrite(sckPin, HIGH);
    digitalWrite(sckPin, LOW);
  }
}
#define DC_CMD()   digitalWrite(dcPin, LOW)
#define DC_DATA()  digitalWrite(dcPin, HIGH)
#define CS_ON()    digitalWrite(csPin, LOW)
#define CS_OFF()   digitalWrite(csPin, HIGH)
#endif

void St7735::command(uint8_t c) { DC_CMD(); byte(c); DC_DATA(); }
void St7735::send(uint16_t c) { byte(c >> 8); byte(c); }
void St7735::sendRepeat(uint16_t c, uint32_t n) {
  uint8_t hi = c >> 8, lo = c;
  for (; n; n--) { byte(hi); byte(lo); }
}
void St7735::end() { CS_OFF(); }

void St7735::window(int16_t x, int16_t y, int16_t ww, int16_t wh) {
  x += x0;
  y += y0;
  CS_ON();
  command(ST77XX_CASET); send(x); send(x + ww - 1);
  command(ST77XX_RASET); send(y); send(y + wh - 1);
  command(ST77XX_RAMWR);
}

// --- Shapes -----------------------------------------------------------------------------------

bool St7735::clip(int16_t &x, int16_t &y, int16_t &rw, int16_t &rh) const {
  if (x < 0) { rw += x; x = 0; }
  if (y < 0) { rh += y; y = 0; }
  if (x + rw > w) rw = w - x;
  if (y + rh > h) rh = h - y;
  return rw > 0 && rh > 0;
}

void St7735::fillRect(int16_t x, int16_t y, int16_t rw, int16_t rh, uint16_t c) {
  if (!clip(x, y, rw, rh)) return;
  window(x, y, rw, rh);
  sendRepeat(c, (uint32_t)rw * rh);
  end();
}

// --- Text and sprites ---------------------------------------------------------------------------

int16_t St7735::fastText(int16_t x, int16_t y, const char *s, bool flash, uint16_t fg, uint16_t bg, uint8_t scale) {
  if (!scale) scale = 1;
  const int16_t cw = 6 * scale, ch = 8 * scale;
  for (;; s++, x += cw) {
    char c = flash ? pgm_read_byte(s) : *s;
    if (!c || x + cw > w) break;
    if (x < 0 || y < 0 || y + ch > h) continue;   // characters are drawn whole or not at all
    const uint8_t *glyph = &FONT5X7[(uint8_t)c * 5];
    uint8_t cols[6];
    for (uint8_t i = 0; i < 5; i++) cols[i] = pgm_read_byte(glyph + i);
    cols[5] = 0;   // 1 px gap between characters
    window(x, y, cw, ch);
    for (int16_t row = 0; row < ch; row++) {
      uint8_t bit = row / scale;
      for (int16_t col = 0; col < cw; col++) send((cols[col / scale] >> bit) & 1 ? fg : bg);
    }
    end();
  }
  return x;
}

void St7735::sprite(int16_t wx, int16_t wy, int16_t ww, int16_t wh, int16_t sx, int16_t sy,
                    const uint16_t *rows, uint8_t sw, uint8_t sh, uint16_t fg, uint16_t bg) {
  if (!clip(wx, wy, ww, wh)) return;
  window(wx, wy, ww, wh);
  for (int16_t py = wy; py < wy + wh; py++) {
    int16_t r = py - sy;
    uint16_t bits = (r >= 0 && r < sh) ? pgm_read_word(&rows[r]) : 0;
    for (int16_t px = wx; px < wx + ww; px++) {
      int16_t c = px - sx;
      send(c >= 0 && c < sw && (bits & (0x8000 >> c)) ? fg : bg);
    }
  }
  end();
}


#endif

#include "gfx.h"

uint16_t hue565(uint8_t hue) {
  uint8_t seg = hue / 43;
  uint8_t f = (hue - seg * 43) * 6;
  uint8_t q = 255 - f;
  switch (seg) {
    case 0:  return color565(255, f, 0);
    case 1:  return color565(q, 255, 0);
    case 2:  return color565(0, 255, f);
    case 3:  return color565(0, q, 255);
    case 4:  return color565(f, 0, 255);
    default: return color565(255, 0, q);
  }
}

uint16_t dim565(uint16_t c, uint8_t k) {
  uint16_t r = (((c >> 11) & 31) * k) >> 8;
  uint16_t g = (((c >> 5) & 63) * k) >> 8;
  uint16_t b = ((c & 31) * k) >> 8;
  return (r << 11) | (g << 5) | b;
}

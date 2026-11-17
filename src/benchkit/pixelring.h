// A NeoPixel ring (or strip): WS2812 / SK6812, RGB or RGBW. Data In through 300-500 ohm to any
// pin, a 22-1000 uF capacitor across 5V/GND at the ring. Each LED draws up to ~60-80 mA at full
// white: a few dim LEDs are fine on the board's 5V, bright = a 5 V supply of its own. Needs the
// Adafruit NeoPixel library: `adafruit/Adafruit NeoPixel` in lib_deps.
//
//   PixelRing<24> ring;
//   setup(): ring.begin(RING_PIN, NEO_GRBW + NEO_KHZ800, 60);   // type (RGBW ring), brightness
//   loop():  ring.set(3, PixelRing<24>::hsv(21845));            // green
//            ring.show();   // sends only if a colour changed
//
// show() switches interrupts off for ~30 us per LED (24 LEDs: ~1 ms), so set() remembers each
// LED's colour and show() does nothing unless one changed. (A servo on Timer1Servo doesn't mind
// the interrupts being off; one on the Servo library twitches.)
//
// The ring arithmetic below (positions between LEDs, distance around the ring, colour shades)
// is plain C++, tested on the computer.
#pragma once
#include <stdint.h>

// Shortest step from LED a to LED b around a ring of n: -n/2 < step <= n/2
inline int8_t ringStep(uint8_t a, uint8_t b, uint8_t n) {
  int16_t d = ((int16_t)b - a + n) % n;
  if (d > n / 2) d -= n;
  return (int8_t)d;
}

// How far LED `led` is from position `pos` (in 1/256ths of an LED: LED k is at k * 256), the
// short way round, in 1/256ths of an LED: 0 .. n * 128.
inline uint16_t ringDistance256(uint8_t led, uint16_t pos, uint8_t n) {
  int32_t full = (int32_t)n * 256;
  int32_t d = (int32_t)led * 256 - pos;
  if (d < 0) d = -d;
  if (d > full - d) d = full - d;
  return (uint16_t)d;
}

// Colouring by distance with a table of `shades` colours, worked out once, instead of a colour
// calculation per LED per update. Shade k stands for this distance (1/256 LED)...
inline uint16_t ringShadeDistance(uint8_t k, uint8_t n, uint8_t shades) {
  return (uint32_t)k * n * 128 / (shades - 1);
}
// ...and an LED at distance d (from ringDistance256) gets the nearest shade.
inline uint8_t ringShade(uint16_t d, uint8_t n, uint8_t shades) {
  uint32_t s = ((uint32_t)d * (shades - 1) + n * 64) / ((uint32_t)n * 128);
  return s > (uint32_t)(shades - 1) ? shades - 1 : (uint8_t)s;
}

#if defined(ARDUINO) && !defined(NATIVE_TEST) && __has_include(<Adafruit_NeoPixel.h>)
#include <Adafruit_NeoPixel.h>

template <uint8_t N>
class PixelRing {
public:
  static const uint8_t count = N;

  // type: NEO_GRBW + NEO_KHZ800 for RGBW rings, NEO_GRB + NEO_KHZ800 for RGB.
  // brightness 0-255 scales every colour. Starts with every LED off.
  void begin(uint8_t pin, uint16_t type = NEO_GRB + NEO_KHZ800, uint8_t brightness = 60) {
    strip.updateType(type);
    strip.updateLength(N);
    strip.setPin(pin);
    strip.begin();
    strip.setBrightness(brightness);
    for (uint8_t i = 0; i < N; i++) shown[i] = 0;
    strip.show();
    dirty = false;
  }

  void set(uint8_t i, uint32_t colour) {
    if (i >= N || colour == shown[i]) return;
    shown[i] = colour;
    strip.setPixelColor(i, colour);
    dirty = true;
  }
  void fill(uint32_t colour) { for (uint8_t i = 0; i < N; i++) set(i, colour); }
  void clear() { fill(0); }
  uint32_t get(uint8_t i) const { return i < N ? shown[i] : 0; }

  bool show() {   // true if it sent (something changed)
    if (!dirty) return false;
    strip.show();
    dirty = false;
    return true;
  }

  // Colours: hsv() is gamma-corrected (hue 0 red, 21845 green, 43690 blue, 65535 back to red)
  static uint32_t hsv(uint16_t hue, uint8_t sat = 255, uint8_t val = 255) {
    return Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(hue, sat, val));
  }
  static uint32_t rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) {
    return Adafruit_NeoPixel::Color(r, g, b, w);
  }

  Adafruit_NeoPixel &lib() { return strip; }   // Adafruit's object, for anything not here

private:
  Adafruit_NeoPixel strip;
  uint32_t shown[N];
  bool dirty = false;
};
#endif

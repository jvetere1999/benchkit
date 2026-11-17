// HC-SR04 ultrasonic distance sensor (VCC 5V, Trig, Echo, GND), 2-400 cm:
//
//   HcSr04 sonar;  sonar.begin(TRIG_PIN, ECHO_PIN);
//   int16_t cm = sonar.cm();        // -1 = no echo (nothing in range, or it pointed at something soft / angled)
//   int16_t cm = sonar.cmMedian();  // middle of 3 readings: ignores a single bad one
//
// A reading waits for the echo: up to maxCm's round trip (~12 ms for the default 200 cm), so
// limit maxCm to what you need. Leave >= 60 ms between readings or an old echo comes back.
#pragma once
#include <stdint.h>

// Echo time (µs) to distance: sound travels ~0.343 mm/µs, there and back
inline int16_t echoToMm(uint32_t us) { return us ? (int16_t)(us * 343 / 2000) : -1; }

class HcSr04 {
public:
  uint16_t maxCm = 200;

  void begin(uint8_t trig, uint8_t echo);
  int16_t mm();
  int16_t cm() { int16_t v = mm(); return v < 0 ? -1 : (v + 5) / 10; }
  int16_t cmMedian();

  static int16_t median3(int16_t a, int16_t b, int16_t c);   // -1s count as "no reading"

private:
  uint8_t trig = 255, echo = 255;
};

// Count seconds on a 4-digit 7-segment display (5461AS, common cathode).
// Wiring: segments a-g, dp to 2-9 (each through 220 Ohm); digits 1-4 to 10-13.
#include <Benchkit.h>

const uint8_t SEGS[8] = {2, 3, 4, 5, 6, 7, 8, 9};
const uint8_t DIGITS[4] = {10, 11, 12, 13};
Seg7x4 four;

void setup() {
  four.begin(SEGS, DIGITS);
}

void loop() {
  four.number(millis() / 1000);
  four.update();             // every loop: lights one digit at a time
}

// A running light on 8 LEDs through one 74HC595.
// Wiring: DS 11, SH_CP 12, ST_CP 8 (chip pins 14, 11, 12); LEDs with 330 Ohm on Q0-Q7.
#include <Benchkit.h>

Shift595<1> sr;

void setup() {
  sr.begin(11, 12, 8);
}

void loop() {
  sr.setByte(0, 1 << ((millis() / 150) % 8));
  sr.write();                // only sends when something changed
}

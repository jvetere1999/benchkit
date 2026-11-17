// Play a short tune on a passive buzzer without stopping the loop.
// Wiring: buzzer from pin 8 through 100 Ohm to GND.
#include <Benchkit.h>

const Note TUNE[] PROGMEM = {{NOTE_C5, 150}, {0, 50}, {NOTE_E5, 150}, {0, 50}, {NOTE_G5, 300}};

Buzzer bz;

void setup() {
  bz.begin(8);
  bz.play(TUNE, sizeof(TUNE) / sizeof(TUNE[0]));
}

void loop() {
  bz.update();
}

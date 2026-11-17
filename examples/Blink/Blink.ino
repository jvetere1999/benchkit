// Blink without delay(), plus a flash on top.
// Wiring: LED + 220 Ohm on pin 13 (or the built-in LED).
#include <Benchkit.h>

Led led;

void setup() {
  led.begin(LED_BUILTIN);
  led.blink(200, 800);       // forever
}

void loop() {
  led.update();              // every loop
}

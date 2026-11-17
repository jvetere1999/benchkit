// A DC motor ramping forwards and back on an L293D.
// Wiring: enable 1 (chip pin 1) to 5, input 1 (pin 2) to 6, input 2 (pin 7) to 7. See motor.h for the rest.
#include <Benchkit.h>

Motor m;

void setup() {
  m.begin(5, 6, 7);
  m.rampMs = 1000;
}

void loop() {
  m.speed((millis() / 4000) % 2 ? 80 : -80);   // switch direction every 4 s, ramping
  m.update();
}

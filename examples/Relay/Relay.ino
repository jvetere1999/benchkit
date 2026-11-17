// Switch a relay on for 2 s whenever the button is tapped.
// Wiring: relay module IN to 7 (most modules switch on with LOW), button on 2 as in the Button example.
#include <Benchkit.h>

Relay relay;
Button btn;

void setup() {
  relay.begin(7, true);      // true = on with LOW
  btn.begin(2);
}

void loop() {
  if (btn.read() == BUTTON_TAP) relay.onFor(2000);
  relay.update();
}

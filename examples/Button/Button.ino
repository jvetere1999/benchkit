// Tap, hold and double tap on one button.
// Wiring: button from pin 2 to 5V, 10 kOhm from pin 2 to GND (pressed = HIGH).
#include <Benchkit.h>

Button btn;
DoubleTap dbl;

void setup() {
  Serial.begin(115200);
  btn.begin(2);
}

void loop() {
  switch (dbl.update(btn.read(), millis())) {
    case BUTTON_TAP:        Serial.println(F("tap")); break;
    case BUTTON_DOUBLE_TAP: Serial.println(F("double tap")); break;
    case BUTTON_HOLD:       Serial.println(F("hold")); break;
    default: break;
  }
}

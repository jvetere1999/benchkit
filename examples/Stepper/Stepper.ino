// 28BYJ-48 stepper: a quarter turn each way, forever.
// Wiring: ULN2003 board IN1-IN4 to 8, 9, 10, 11; board + to 5V, - to GND.
#include <Benchkit.h>

Stepper28BYJ st;

void setup() {
  st.begin(8, 9, 10, 11);
  st.rpm(10);
}

void loop() {
  static bool forwards = true;
  if (!st.busy()) {
    st.move(forwards ? Stepper28BYJ::TURN / 4 : -Stepper28BYJ::TURN / 4);
    forwards = !forwards;
  }
  st.update();
}

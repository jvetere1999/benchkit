// "[  1:23.456] " (minutes:seconds since power-on). Start every serial event line with it;
// a log viewer can split it off into its own column.
//
//   stamp(); Serial.println(F("Button tapped"));
//   stamp(myLog);                               // any Print: Serial1, a TeeLog...
#pragma once
#include <Arduino.h>

inline void stamp(Print &out = Serial) {
  unsigned long ms = millis();
  char buf[16];
  snprintf_P(buf, sizeof(buf), PSTR("[%3lu:%02lu.%03lu] "), ms / 60000, (ms / 1000) % 60, ms % 1000);
  out.print(buf);
}

#include "font5x7.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif
#ifndef PROGMEM
#define PROGMEM
#endif
#include "glcdfont.inc"
const uint8_t FONT5X7[] PROGMEM = GLCDFONT_DATA;

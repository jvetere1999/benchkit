// A fake Arduino for the native tests: pins, analog values and time are plain variables the
// tests set and read, and every pin write is logged. Only what Benchkit uses is here.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define MSBFIRST 1
#define LSBFIRST 0
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#define PROGMEM
#define PSTR(s) (s)
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#define pgm_read_word(p) (*(const uint16_t *)(p))
#define snprintf_P snprintf
#define strlen_P strlen
class __FlashStringHelper;
#define F(s) ((const __FlashStringHelper *)(s))
typedef uint8_t byte;

namespace fake {
extern uint32_t us;                 // the clock, in microseconds
extern uint8_t level[64];           // pin levels: written by the code, or set by a test (inputs)
extern uint8_t mode[64];
extern int16_t analog[64];          // analogRead values
extern int16_t pwm[64];             // last analogWrite per pin (-1 = never)
extern uint32_t pulse;              // what pulseIn returns
extern std::string log;             // "w9=1 " for digitalWrite, "a9=128 ", "t8=440 ", "n8 "
void reset();
}

inline unsigned long millis() { return fake::us / 1000; }
inline unsigned long micros() { return fake::us; }
inline void delay(unsigned long ms) { fake::us += ms * 1000; }
inline void delayMicroseconds(unsigned int u) { fake::us += u; }
inline void pinMode(uint8_t p, uint8_t m) { fake::mode[p] = m; if (m == INPUT_PULLUP) fake::level[p] = HIGH; }
inline int digitalRead(uint8_t p) { return fake::level[p]; }
void digitalWrite(uint8_t p, uint8_t v);
void analogWrite(uint8_t p, int v);
inline int analogRead(uint8_t p) { return fake::analog[p]; }
void tone(uint8_t p, unsigned int hz, unsigned long ms = 0);
void noTone(uint8_t p);
void shiftOut(uint8_t data, uint8_t clock, uint8_t order, uint8_t value);
inline unsigned long pulseIn(uint8_t, uint8_t, unsigned long = 1000000) { return fake::pulse; }
inline void noInterrupts() {}
inline void interrupts() {}

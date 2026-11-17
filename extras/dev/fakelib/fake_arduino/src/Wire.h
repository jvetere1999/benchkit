// Fake Wire for the native tests: logs each transmission as "[3c: 00 ae ...] ".
#pragma once
#include <Arduino.h>

class TwoWire {
public:
  void begin() {}
  void setClock(uint32_t) {}
  void beginTransmission(uint8_t a);
  size_t write(uint8_t b);
  uint8_t endTransmission(bool = true);
  uint8_t present = 0x3C;      // the address that answers (0 = none)
  std::string log;
  int bytes = 0;               // data bytes sent, all transmissions
private:
  uint8_t addr = 0;
};
extern TwoWire Wire;

#include "nrf24.h"
#if defined(ARDUINO) && !defined(NATIVE_TEST) && __has_include(<RF24.h>)
#include <SPI.h>

void Nrf24::park(uint8_t ce, uint8_t csn) {
  pinMode(csn, OUTPUT);
  digitalWrite(csn, HIGH);
  pinMode(ce, OUTPUT);
  digitalWrite(ce, LOW);
}

bool Nrf24::begin(uint8_t ce, uint8_t csn, uint8_t channel, uint16_t kbps, uint8_t power,
                  uint8_t payloadSize) {
  ok = rf.begin(ce, csn) && rf.isChipConnected();
  if (!ok) return false;
  rf.setChannel(channel);
  rf24_datarate_e want = kbps == 250 ? RF24_250KBPS : kbps == 2000 ? RF24_2MBPS : RF24_1MBPS;
  rate = rf.setDataRate(want) && rf.getDataRate() == want;
  rf.setPALevel((rf24_pa_dbm_e)(power > 3 ? 3 : power));
  rf.setPayloadSize(payloadSize);
  return true;
}

void Nrf24::sender(const char *address, uint8_t retryDelay, uint8_t retries) {
  if (!ok) return;
  rf.setRetries(retryDelay, retries);
  rf.openWritingPipe((const uint8_t *)address);
  rf.stopListening();
}

bool Nrf24::send(const void *data, uint8_t size) {
  if (!ok) return false;
  bool acked = rf.write(data, size);
  acks.add(acked);
  return acked;
}

void Nrf24::receiver(const char *address) {
  if (!ok) return;
  rf.openReadingPipe(1, (const uint8_t *)address);
  rf.startListening();
}

bool Nrf24::receive(void *data, uint8_t size) {
  if (!ok || !rf.available()) return false;
  rf.read(data, size);
  return true;
}

void Nrf24::sampleSignal() {
  if (!ok || micros() - lastSampleUs < 2000) return;
  lastSampleUs = micros();
  samples++;
  if (rf.testRPD()) hits++;
}

void Nrf24::signalReset() { hits = samples = 0; }

uint8_t Nrf24::signalPercent() {
  uint8_t pct = samples ? (uint32_t)hits * 100 / samples : 0;
  signalReset();
  if (ok) {   // the signal flag sticks once a packet arrives, until listening restarts
    rf.stopListening();
    rf.startListening();
  }
  return pct;
}

bool Nrf24::signalOn(uint8_t channel) {
  if (!ok) return false;
  rf.setChannel(channel);
  rf.startListening();
  delayMicroseconds(128);   // long enough for the chip to measure the level
  rf.stopListening();
  return rf.testRPD();
}
#endif

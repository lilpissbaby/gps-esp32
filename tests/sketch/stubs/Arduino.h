#pragma once
// Imitación mínima del entorno Arduino-ESP32 para probar el sketch en el PC.
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <deque>
#define INPUT_PULLUP 0x05
#define LOW 0
#define HIGH 1
#define SERIAL_8N1 0x800001c
#define FILE_WRITE "w"
#define FILE_READ "r"
struct String {
  std::string s;
  String(const char* c = "") : s(c) {}
  bool operator==(const char* o) const { return s == o; }
};
struct HardwareSerial {
  unsigned long baud = 0;  // última velocidad pedida
  void begin(unsigned long b, uint32_t = 0, int8_t = -1, int8_t = -1) { baud = b; }
  void updateBaudRate(unsigned long b) { baud = b; }
  std::deque<uint8_t> rx;        // bytes que "manda" el GPS imitado
  std::vector<uint8_t> written;  // bytes que el sketch le manda al GPS
  int available() { return (int)rx.size(); }
  int read() { int c = rx.front(); rx.pop_front(); return c; }
  size_t write(const uint8_t* d, size_t n) { written.insert(written.end(), d, d + n); return n; }
  void flush() {}
  size_t println(const char*) { return 0; }
  size_t printf(const char*, ...) { return 0; }
};
extern HardwareSerial Serial, Serial2;
extern uint32_t g_millis;  // reloj simulado: delay() lo avanza
inline unsigned long millis() { return g_millis; }
inline void delay(unsigned long ms) { g_millis += ms; }
inline void pinMode(uint8_t, uint8_t) {}
extern int g_pinLevel[40];  // HIGH = botón suelto, LOW = pulsado
inline int digitalRead(uint8_t pin) { return g_pinLevel[pin]; }
inline uint32_t analogReadMilliVolts(uint8_t) { return 0; }

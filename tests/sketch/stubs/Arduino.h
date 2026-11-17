#pragma once
// Imitación mínima del entorno Arduino-ESP32 para probar el sketch en el PC.
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
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
  void begin(unsigned long, uint32_t = 0, int8_t = -1, int8_t = -1) {}
  int available() { return 0; }
  int read() { return 0; }
  size_t println(const char*) { return 0; }
  size_t printf(const char*, ...) { return 0; }
};
extern HardwareSerial Serial, Serial2;
extern uint32_t g_millis;  // reloj simulado: delay() lo avanza
inline unsigned long millis() { return g_millis; }
inline void delay(unsigned long ms) { g_millis += ms; }
inline void pinMode(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t) { return HIGH; }  // ningún botón pulsado
inline uint32_t analogReadMilliVolts(uint8_t) { return 0; }

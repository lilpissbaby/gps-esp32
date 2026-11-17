#pragma once
#include "Arduino.h"
struct SPIClass {};
extern SPIClass SPI;
struct File {
  explicit operator bool() const { return true; }
  size_t print(const char*) { return 0; }
  void close() {}
  String readString() { return String("GPS ESP32 OK"); }
};
namespace fs {
struct FS {
  File open(const char*, const char*) { return File(); }
  bool exists(const char*);
};
}  // namespace fs
struct SDFS : fs::FS {
  bool begin(uint8_t, SPIClass&, uint32_t);
};
extern SDFS SD;
extern bool g_sdPresent, g_sdHasJpg;

#pragma once
#include <stdint.h>
extern uint32_t g_gpsChars, g_gpsSentences;  // lo que "ha recibido" el GPS imitado
struct TinyGPSLocation { bool isValid() const {return false;} double lat(){return 0;} double lng(){return 0;} };
struct TinyGPSInteger { bool isValid() const {return false;} uint32_t value(){return 0;} };
struct TinyGPSPlus {
  bool encode(char){return false;}
  uint32_t charsProcessed() const {return g_gpsChars;}
  uint32_t passedChecksum() const {return g_gpsSentences;}
  uint32_t failedChecksum() const {return 0;}
  TinyGPSLocation location; TinyGPSInteger satellites;
};

#pragma once
#include <stdint.h>
struct TinyGPSLocation { bool isValid() const {return false;} double lat(){return 0;} double lng(){return 0;} };
struct TinyGPSInteger { bool isValid() const {return false;} uint32_t value(){return 0;} };
struct TinyGPSPlus { bool encode(char){return false;} uint32_t charsProcessed() const {return 0;} TinyGPSLocation location; TinyGPSInteger satellites; };

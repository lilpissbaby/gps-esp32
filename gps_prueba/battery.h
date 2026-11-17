#pragma once
#include <stdint.h>

// Voltaje real de la batería a partir de los mV que lee el pin y la relación del divisor
// (2.0 con dos resistencias iguales).
float batteryVolts(uint32_t pinMillivolts, float dividerRatio);

// Porcentaje aproximado de una LiPo (0-100). Devuelve -1 si el voltaje es tan bajo
// que no puede ser una batería conectada (divisor sin montar): "sin medir".
int batteryPercent(float volts);

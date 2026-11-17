#pragma once
#include <stdint.h>

// Voltaje real de la batería a partir de los mV que lee el pin y la relación del divisor
// (2.0 con dos resistencias iguales).
float batteryVolts(uint32_t pinMillivolts, float dividerRatio);

// Porcentaje aproximado de una LiPo (0-100). Devuelve -1 ("sin medir") si el voltaje
// no puede ser de una LiPo: muy bajo (divisor sin montar) o por encima de 4,4 V (pin al aire).
int batteryPercent(float volts);

#include "battery.h"

float batteryVolts(uint32_t pinMillivolts, float dividerRatio) {
  return pinMillivolts * dividerRatio / 1000.0f;
}

namespace {
struct Point { float volts; int percent; };
// Curva de descarga aproximada de una LiPo de 3,7 V, de más a menos.
const Point kCurve[] = {
    {4.20f, 100}, {4.10f, 90}, {4.00f, 80}, {3.90f, 65}, {3.80f, 50}, {3.75f, 40},
    {3.70f, 30},  {3.65f, 20}, {3.60f, 10}, {3.50f, 5},  {3.30f, 0},
};
const int kPoints = sizeof(kCurve) / sizeof(kCurve[0]);
const float kNotConnected = 2.5f;  // por debajo: divisor sin montar
const float kImpossiblyHigh = 4.4f;  // por encima: pin al aire, una LiPo no llega
}  // namespace

int batteryPercent(float volts) {
  if (volts < kNotConnected || volts > kImpossiblyHigh) return -1;
  if (volts >= kCurve[0].volts) return 100;
  if (volts <= kCurve[kPoints - 1].volts) return 0;
  for (int i = 1; i < kPoints; i++) {
    const Point& hi = kCurve[i - 1];
    const Point& lo = kCurve[i];
    if (volts >= lo.volts) {
      float t = (volts - lo.volts) / (hi.volts - lo.volts);
      return static_cast<int>(lo.percent + t * (hi.percent - lo.percent) + 0.5f);
    }
  }
  return 0;
}

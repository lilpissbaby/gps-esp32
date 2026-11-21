// Tests de la lógica pura del programa de prueba. Se compilan en el PC con g++ y Unity:
//   make -C tests
#include <unity.h>
#include "battery.h"
#include "debouncer.h"
#include "gpsdiag.h"

void setUp() {}
void tearDown() {}

void test_bateria_voltios() { TEST_ASSERT_FLOAT_WITHIN(0.001, 4.0, batteryVolts(2000, 2.0)); }
void test_bateria_llena() { TEST_ASSERT_EQUAL(100, batteryPercent(4.20)); }
void test_bateria_por_encima() { TEST_ASSERT_EQUAL(100, batteryPercent(4.35)); }
void test_bateria_vacia() { TEST_ASSERT_EQUAL(0, batteryPercent(3.30)); }
void test_bateria_media() { TEST_ASSERT_EQUAL(40, batteryPercent(3.75)); }
void test_bateria_interpolada() { TEST_ASSERT_EQUAL(45, batteryPercent(3.775)); }
void test_bateria_sin_divisor() { TEST_ASSERT_EQUAL(-1, batteryPercent(1.0)); }
// Pin al aire que deriva hacia arriba: una LiPo nunca pasa de ~4,25 V.
void test_bateria_flotante_alta() { TEST_ASSERT_EQUAL(-1, batteryPercent(4.6)); }

void test_boton_pulsacion_estable() {      // pulsado 30 ms seguidos → 1 evento
  Debouncer d(30);
  TEST_ASSERT_FALSE(d.update(true, 0));
  TEST_ASSERT_TRUE(d.update(true, 30));
  TEST_ASSERT_TRUE(d.isPressed());
}

void test_boton_rebote_no_cuenta() {       // 0-1-0-1 en menos de 30 ms → ningún evento
  Debouncer d(30);
  TEST_ASSERT_FALSE(d.update(true, 0));
  TEST_ASSERT_FALSE(d.update(false, 5));
  TEST_ASSERT_FALSE(d.update(true, 10));
  TEST_ASSERT_FALSE(d.update(true, 25));
}

void test_boton_mantenido_un_evento() {    // mantenido 2 s → solo 1 evento; soltar y volver → otro
  Debouncer d(30);
  d.update(true, 0);
  TEST_ASSERT_TRUE(d.update(true, 30));
  TEST_ASSERT_FALSE(d.update(true, 2000));
  d.update(false, 2010);
  d.update(false, 2040);
  d.update(true, 2050);
  TEST_ASSERT_TRUE(d.update(true, 2080));
}

// gpsState(caracteres, frases NMEA válidas, hay posición, ms escuchando)
void test_gps_esperando() { TEST_ASSERT_EQUAL(GpsState::Waiting, gpsState(0, 0, false, 2000)); }
void test_gps_sin_datos_tras_5s() { TEST_ASSERT_EQUAL(GpsState::NoData, gpsState(0, 0, false, 5000)); }
// Un cable suelto o tocado mete algún carácter de ruido: eso no son datos del GPS.
void test_gps_ruido_no_cuenta_como_datos() { TEST_ASSERT_EQUAL(GpsState::NoData, gpsState(2, 0, false, 183000)); }
// Muchos caracteres pero ninguna frase válida: el GPS habla a otra velocidad (baudios).
void test_gps_basura_es_velocidad() { TEST_ASSERT_EQUAL(GpsState::BadData, gpsState(5000, 0, false, 10000)); }
void test_gps_sin_fix() { TEST_ASSERT_EQUAL(GpsState::NoFix, gpsState(1200, 30, false, 60000)); }
void test_gps_fix() { TEST_ASSERT_EQUAL(GpsState::Fix, gpsState(1200, 30, true, 60000)); }

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_bateria_voltios);
  RUN_TEST(test_bateria_llena);
  RUN_TEST(test_bateria_por_encima);
  RUN_TEST(test_bateria_vacia);
  RUN_TEST(test_bateria_media);
  RUN_TEST(test_bateria_interpolada);
  RUN_TEST(test_bateria_sin_divisor);
  RUN_TEST(test_bateria_flotante_alta);
  RUN_TEST(test_boton_pulsacion_estable);
  RUN_TEST(test_boton_rebote_no_cuenta);
  RUN_TEST(test_boton_mantenido_un_evento);
  RUN_TEST(test_gps_esperando);
  RUN_TEST(test_gps_sin_datos_tras_5s);
  RUN_TEST(test_gps_ruido_no_cuenta_como_datos);
  RUN_TEST(test_gps_basura_es_velocidad);
  RUN_TEST(test_gps_sin_fix);
  RUN_TEST(test_gps_fix);
  return UNITY_END();
}

// Tests de la lógica pura del programa de prueba. Se compilan en el PC con g++ y Unity:
//   make -C tests
#include <string.h>
#include <unity.h>
#include "battery.h"
#include "debouncer.h"
#include "gpsdiag.h"
#include "baudscan.h"
#include "ubx.h"

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

// gpsState(caracteres por segundo, frases NMEA válidas, tramas UBX válidas, hay posición, ms)
void test_gps_esperando() { TEST_ASSERT_EQUAL(GpsState::Waiting, gpsState(0, 0, 0, false, 2000)); }
void test_gps_sin_datos_tras_5s() { TEST_ASSERT_EQUAL(GpsState::NoData, gpsState(0, 0, 0, false, 5000)); }
// Un cable suelto o tocado mete algún carácter suelto de ruido: eso no son datos del GPS.
void test_gps_ruido_no_cuenta_como_datos() { TEST_ASSERT_EQUAL(GpsState::NoData, gpsState(1, 0, 0, false, 183000)); }
// Llega señal de verdad (muchos caracteres por segundo) pero ninguna frase: datos rotos.
void test_gps_senal_rota() { TEST_ASSERT_EQUAL(GpsState::BadData, gpsState(500, 0, 0, false, 10000)); }
// Tramas UBX válidas y ninguna frase NMEA: el GPS está configurado solo en binario.
void test_gps_solo_ubx() { TEST_ASSERT_EQUAL(GpsState::UbxOnly, gpsState(900, 0, 5, false, 10000)); }
void test_gps_sin_fix() { TEST_ASSERT_EQUAL(GpsState::NoFix, gpsState(480, 30, 0, false, 60000)); }
void test_gps_fix() { TEST_ASSERT_EQUAL(GpsState::Fix, gpsState(480, 30, 0, true, 60000)); }

// UBX: el protocolo binario de u-blox. ACK-ACK de CFG-PRT, ejemplo conocido del manual.
static const uint8_t kAck[] = {0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x00, 0x0E, 0x37};
void test_ubx_reconoce_trama_valida() {
  UbxDetector d;
  int done = 0;
  for (uint8_t c : kAck) done += d.feed(c);
  TEST_ASSERT_EQUAL(1, done);
  TEST_ASSERT_EQUAL_UINT32(1, d.validFrames());
}
void test_ubx_rechaza_suma_de_control_mala() {
  UbxDetector d;
  uint8_t bad[sizeof(kAck)];
  memcpy(bad, kAck, sizeof(kAck));
  bad[9] = 0x38;
  for (uint8_t c : bad) d.feed(c);
  TEST_ASSERT_EQUAL_UINT32(0, d.validFrames());
}
void test_ubx_encuentra_la_trama_entre_basura() {
  UbxDetector d;
  const uint8_t junk[] = {0x13, 0xB5, 0x00, 0xB5, 0xB5};
  for (uint8_t c : junk) d.feed(c);
  for (uint8_t c : kAck) d.feed(c);
  for (uint8_t c : junk) d.feed(c);
  TEST_ASSERT_EQUAL_UINT32(1, d.validFrames());
}
void test_ubx_nmea_no_cuenta() {
  UbxDetector d;
  const char* nmea = "$GPGGA,,,,,,0,00,99.99,,,,,,*48\r\n";
  for (const char* c = nmea; *c; c++) d.feed((uint8_t)*c);
  TEST_ASSERT_EQUAL_UINT32(0, d.validFrames());
}
// Orden UBX-CFG-CFG "borrar la configuración guardada y cargar la de fábrica".
void test_ubx_orden_de_fabrica() {
  const uint8_t expected[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x17, 0x2F, 0xAE};
  uint8_t out[32];
  TEST_ASSERT_EQUAL(sizeof(expected), ubxFactoryReset(out, sizeof(out)));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, sizeof(expected));
}

// BaudScanner: prueba velocidades hasta que llega una frase NMEA válida.
static const uint32_t kBauds[] = {9600, 38400, 19200};
void test_baud_empieza_por_la_primera() {
  BaudScanner b(kBauds, 3, 2500);
  b.start(1000);
  TEST_ASSERT_EQUAL_UINT32(9600, b.baud());
  TEST_ASSERT_FALSE(b.locked());
}
void test_baud_cambia_si_no_llegan_frases() {
  BaudScanner b(kBauds, 3, 2500);
  b.start(1000);
  TEST_ASSERT_FALSE(b.update(3400, 0));   // aún no han pasado 2,5 s
  TEST_ASSERT_TRUE(b.update(3500, 0));    // toca cambiar
  TEST_ASSERT_EQUAL_UINT32(38400, b.baud());
}
void test_baud_vuelve_a_empezar_tras_la_ultima() {
  BaudScanner b(kBauds, 3, 2500);
  b.start(0);
  b.update(2500, 0);
  b.update(5000, 0);
  TEST_ASSERT_TRUE(b.update(7500, 0));
  TEST_ASSERT_EQUAL_UINT32(9600, b.baud());
}
void test_baud_se_queda_con_la_buena() {
  BaudScanner b(kBauds, 3, 2500);
  b.start(0);
  b.update(2500, 0);                      // pasa a 38400
  TEST_ASSERT_FALSE(b.update(3000, 4));   // llegan frases válidas
  TEST_ASSERT_TRUE(b.locked());
  TEST_ASSERT_FALSE(b.update(60000, 4));  // y ya no se mueve
  TEST_ASSERT_EQUAL_UINT32(38400, b.baud());
}
// Las frases contadas antes de empezar (a otra velocidad) no cuentan como acierto.
void test_baud_no_cuenta_frases_de_antes() {
  BaudScanner c(kBauds, 3, 2500);
  c.start(0, 7);  // ya había 7 frases contadas antes de empezar
  TEST_ASSERT_FALSE(c.update(1000, 7));
  TEST_ASSERT_FALSE(c.locked());
}

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
  RUN_TEST(test_gps_senal_rota);
  RUN_TEST(test_gps_solo_ubx);
  RUN_TEST(test_gps_sin_fix);
  RUN_TEST(test_gps_fix);
  RUN_TEST(test_ubx_reconoce_trama_valida);
  RUN_TEST(test_ubx_rechaza_suma_de_control_mala);
  RUN_TEST(test_ubx_encuentra_la_trama_entre_basura);
  RUN_TEST(test_ubx_nmea_no_cuenta);
  RUN_TEST(test_ubx_orden_de_fabrica);
  RUN_TEST(test_baud_empieza_por_la_primera);
  RUN_TEST(test_baud_cambia_si_no_llegan_frases);
  RUN_TEST(test_baud_vuelve_a_empezar_tras_la_ultima);
  RUN_TEST(test_baud_se_queda_con_la_buena);
  RUN_TEST(test_baud_no_cuenta_frases_de_antes);
  return UNITY_END();
}

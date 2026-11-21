// Prueba el sketch completo en el PC, con imitaciones de Arduino, SD, LovyanGFX y TinyGPS++.
// Comprueba qué textos acaba dibujando en pantalla según el tiempo y lo que hay en la SD.
#include <Arduino.h>
#include <algorithm>
#include <unity.h>

#include "gps_prueba.ino"

// ---- Estado de las imitaciones ----
HardwareSerial Serial, Serial2;
SPIClass SPI;
SDFS SD;
const IFont fonts::Font2, fonts::Font4;
uint32_t g_millis = 0;
bool g_sdPresent = true;
bool g_sdHasJpg = true;
uint32_t g_gpsChars = 0, g_gpsSentences = 0;
int g_pinLevel[40];
std::vector<std::string> g_drawn;
bool fs::FS::exists(const char*) { return g_sdHasJpg; }
bool SDFS::begin(uint8_t, SPIClass&, uint32_t) { return g_sdPresent; }

static bool drew(const char* text) {
  return std::find(g_drawn.begin(), g_drawn.end(), text) != g_drawn.end();
}

void setUp() {}
void tearDown() {}

// Con prueba.jpg en la SD el arranque tarda más de 5 s. Un GPS bien cableado aún no ha
// mandado nada en ese momento y no debe salir el aviso de cables cruzados.
void test_arranque_con_jpg_no_avisa_de_tx_rx() {
  for (int& l : g_pinLevel) l = HIGH;  // ningún botón pulsado
  g_millis = 0;
  g_sdHasJpg = true;
  setup();
  g_drawn.clear();
  loop();
  TEST_ASSERT_FALSE(drew("Sin datos: revisa TX/RX"));
  TEST_ASSERT_TRUE(drew("Esperando datos..."));
}

// Si de verdad no llega nada, el aviso sí debe salir a los 5 s de empezar a escuchar al GPS.
void test_gps_mudo_avisa_a_los_5_s() {
  g_millis += 5500;
  g_drawn.clear();
  loop();
  TEST_ASSERT_TRUE(drew("Sin datos: revisa TX/RX"));
}

// Lo que pasó en la protoboard: GPS sin datos y 2 caracteres de ruido al tocar los cables.
// Debe seguir diciendo que no llegan datos, no "Sin posicion todavia".
void test_ruido_en_el_cable_no_engana() {
  g_millis += 180000;
  g_gpsChars = 2;
  g_gpsSentences = 0;
  g_drawn.clear();
  loop();
  TEST_ASSERT_TRUE(drew("Sin datos: revisa TX/RX"));
  TEST_ASSERT_FALSE(drew("Sin posicion todavia"));
}

// Llega mucha basura y ninguna frase: a los 2,5 s prueba otra velocidad y lo dice en pantalla.
void test_prueba_otra_velocidad_si_llega_basura() {
  g_gpsChars = 5000;
  g_gpsSentences = 0;
  g_millis += 2600;
  unsigned long before = Serial2.baud;
  g_drawn.clear();
  loop();
  TEST_ASSERT_NOT_EQUAL(before, Serial2.baud);
  char expected[32];
  snprintf(expected, sizeof(expected), "probando %lu", Serial2.baud);
  TEST_ASSERT_TRUE(drew(expected));
  TEST_ASSERT_TRUE(drew("Datos con errores"));
}

// En cuanto llegan frases válidas se queda con esa velocidad.
void test_se_queda_con_la_velocidad_que_funciona() {
  g_gpsSentences = 12;
  g_millis += 2600;
  unsigned long found = Serial2.baud;
  g_drawn.clear();
  loop();
  g_millis += 10000;
  loop();
  TEST_ASSERT_EQUAL(found, Serial2.baud);
  char expected[32];
  snprintf(expected, sizeof(expected), "%lu detectada", found);
  TEST_ASSERT_TRUE(drew(expected));
}

// Lo que sospechamos en la protoboard: un GPS que estuvo en un dron y quedó configurado para
// hablar solo en binario (UBX). Debe detectarlo, decir que pulses OK y, al pulsarlo, mandar
// la orden de fábrica y volver a escuchar desde 9600.
void test_gps_en_binario_se_puede_dejar_de_fabrica() {
  static const uint8_t ack[] = {0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x00, 0x0E, 0x37};
  g_gpsSentences = 12;  // las frases de antes no cuentan: empieza un GPS "nuevo"
  setup();
  g_millis += 2600;
  loop();  // sin nada a 9600: pasa a 38400
  g_millis += 3500;
  for (int i = 0; i < 3; i++) Serial2.rx.insert(Serial2.rx.end(), ack, ack + sizeof(ack));
  g_drawn.clear();
  loop();
  TEST_ASSERT_TRUE(drew("UBX: pulsa OK=fabrica"));
  const unsigned long ubxBaud = Serial2.baud;

  Serial2.written.clear();
  g_pinLevel[PIN_BTN_OK] = LOW;
  for (int i = 0; i < 4; i++) { g_millis += 20; loop(); }
  g_pinLevel[PIN_BTN_OK] = HIGH;
  const uint8_t reset[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x17, 0x2F, 0xAE};
  TEST_ASSERT_EQUAL(sizeof(reset), Serial2.written.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(reset, Serial2.written.data(), sizeof(reset));
  TEST_ASSERT_EQUAL(38400UL, ubxBaud);  // la orden se manda a la velocidad a la que habla
  TEST_ASSERT_EQUAL(9600UL, Serial2.baud);  // y luego vuelve a escuchar desde 9600
  g_millis += 600;
  g_drawn.clear();
  loop();
  TEST_ASSERT_FALSE(drew("UBX: pulsa OK=fabrica"));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_arranque_con_jpg_no_avisa_de_tx_rx);
  RUN_TEST(test_gps_mudo_avisa_a_los_5_s);
  RUN_TEST(test_ruido_en_el_cable_no_engana);
  RUN_TEST(test_prueba_otra_velocidad_si_llega_basura);
  RUN_TEST(test_se_queda_con_la_velocidad_que_funciona);
  RUN_TEST(test_gps_en_binario_se_puede_dejar_de_fabrica);
  return UNITY_END();
}

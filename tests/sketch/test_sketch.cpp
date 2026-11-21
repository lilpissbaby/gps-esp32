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

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_arranque_con_jpg_no_avisa_de_tx_rx);
  RUN_TEST(test_gps_mudo_avisa_a_los_5_s);
  RUN_TEST(test_ruido_en_el_cable_no_engana);
  return UNITY_END();
}

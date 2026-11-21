// Programa de prueba del GPS portátil: comprueba pantalla, microSD, GPS, botones y batería
// montados en la protoboard. Todo lo que ve la pantalla sale también por el Monitor Serie
// a 115200 baudios.
//
// Librerías (Gestor de librerías del IDE de Arduino): LovyanGFX y TinyGPSPlus.
// Placa: "ESP32 Dev Module".

#include "lgfx_config.h"
#include <TinyGPS++.h>

#include "battery.h"
#include "debouncer.h"
#include "gpsdiag.h"
#include "baudscan.h"
#include "ubx.h"
#include "pins.h"

LGFX lcd;
TinyGPSPlus gps;

struct Button {
  const char* name;
  int pin;
  Debouncer debouncer;
  uint32_t count;
};

Button buttons[] = {
    {"+", PIN_BTN_ZOOM_IN, Debouncer(30), 0},
    {"-", PIN_BTN_ZOOM_OUT, Debouncer(30), 0},
    {"OK", PIN_BTN_OK, Debouncer(30), 0},
    {"<-", PIN_BTN_BACK, Debouncer(30), 0},
};
const char* lastButton = "ninguno";

uint32_t gpsStartMs = 0;  // cuándo empezamos a escuchar al GPS
BaudScanner gpsBaud(GPS_BAUDS, sizeof(GPS_BAUDS) / sizeof(GPS_BAUDS[0]), 2500);
UbxDetector ubx;                // reconoce tramas binarias UBX en lo que manda el GPS
uint32_t sentencesAtStart = 0;  // frases y tramas contadas antes de (re)empezar a escuchar
uint32_t ubxAtStart = 0;
uint32_t charsPerSecond = 0;    // señal que llega, para distinguirla del ruido de un cable

uint32_t gpsSentences() { return gps.passedChecksum() - sentencesAtStart; }
uint32_t gpsUbxFrames() { return ubx.validFrames() - ubxAtStart; }

// Empieza (o vuelve a empezar) a escuchar al GPS desde la primera velocidad.
void startGpsListening(uint32_t now) {
  sentencesAtStart = gps.passedChecksum();
  ubxAtStart = ubx.validFrames();
  gpsStartMs = now;
  gpsBaud.start(now, 0);
}

GpsState currentGpsState(uint32_t now) {
  return gpsState(charsPerSecond, gpsSentences(), gpsUbxFrames(), gps.location.isValid(),
                  now - gpsStartMs);
}

enum class SdState { NotFound, WriteError, Ok };
SdState sdState = SdState::NotFound;
const char* jpgState = "sin tarjeta";

// ---------- Dibujo ----------

const int VALUE_X = 66;  // donde empieza la columna de valores

void drawLabel(const char* text, int y) {
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.drawString(text, 4, y);
}

void drawValue(const char* text, int y, uint16_t color) {  // uint16_t = RGB565 en LovyanGFX
  lcd.fillRect(VALUE_X, y, 240 - VALUE_X, 18, TFT_BLACK);
  lcd.setTextColor(color, TFT_BLACK);
  lcd.drawString(text, VALUE_X, y);
}

void showColorBars() {
  struct Bar { const char* name; uint16_t color; uint16_t text; };  // uint32_t seria RGB888
  const Bar bars[] = {
      {"ROJO", TFT_RED, TFT_WHITE},     {"VERDE", TFT_GREEN, TFT_BLACK},
      {"AZUL", TFT_BLUE, TFT_WHITE},    {"BLANCO", TFT_WHITE, TFT_BLACK},
      {"NEGRO", TFT_BLACK, TFT_WHITE},
  };
  const int h = 320 / 5;
  lcd.setFont(&fonts::Font4);
  lcd.setTextDatum(textdatum_t::middle_center);
  for (int i = 0; i < 5; i++) {
    lcd.fillRect(0, i * h, 240, h, bars[i].color);
    lcd.setTextColor(bars[i].text, bars[i].color);
    lcd.drawString(bars[i].name, 120, i * h + h / 2);
  }
  lcd.setFont(&fonts::Font2);
  lcd.setTextDatum(textdatum_t::top_left);
  lcd.setTextColor(TFT_WHITE, TFT_RED);
  lcd.drawString("^ ARRIBA", 4, 4);
  Serial.println("Pantalla: barras ROJO, VERDE, AZUL, BLANCO, NEGRO (de arriba abajo)");
}

void drawLayout() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextDatum(textdatum_t::top_left);
  lcd.setFont(&fonts::Font4);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("Prueba GPS", 4, 4);
  lcd.setFont(&fonts::Font2);
  drawLabel("Pantalla", 36);
  drawValue("OK si lees esto", 36, TFT_GREEN);
  drawLabel("SD", 58);
  drawLabel("JPG", 80);
  drawLabel("GPS", 108);
  drawLabel("Datos", 128);
  drawLabel("Lat", 148);
  drawLabel("Lon", 168);
  drawLabel("Botones", 196);
  drawLabel("Ultimo", 216);
  drawLabel("Bateria", 244);
  drawLabel("Tiempo", 272);
  drawLabel("Baudios", 296);
}

// ---------- Pruebas ----------

void testSd() {
  if (!SD.begin(PIN_SD_CS, SPI, 4000000)) {
    sdState = SdState::NotFound;
    jpgState = "sin tarjeta";
    Serial.println("SD: no detectada");
    return;
  }
  const char* kText = "GPS ESP32 OK";
  bool ok = false;
  File f = SD.open("/prueba.txt", FILE_WRITE);
  if (f) {
    f.print(kText);
    f.close();
    f = SD.open("/prueba.txt", FILE_READ);
    if (f) {
      ok = f.readString() == kText;
      f.close();
    }
  }
  sdState = ok ? SdState::Ok : SdState::WriteError;
  Serial.println(ok ? "SD: OK (escribe y lee)" : "SD: detectada pero falla al escribir/leer");

  if (!SD.exists("/prueba.jpg")) {
    jpgState = "no hay prueba.jpg";
    return;
  }
  lcd.fillScreen(TFT_BLACK);
  bool drawn = lcd.drawJpgFile(SD, "/prueba.jpg", 0, 0);
  jpgState = drawn ? "dibujado" : "no se pudo dibujar";
  Serial.printf("JPG: %s\n", jpgState);
  delay(3000);
}

void drawSdValues() {
  switch (sdState) {
    case SdState::Ok:         drawValue("OK", 58, TFT_GREEN); break;
    case SdState::WriteError: drawValue("Falla al escribir", 58, TFT_ORANGE); break;
    case SdState::NotFound:   drawValue("No detectada", 58, TFT_RED); break;
  }
  drawValue(jpgState, 80, strcmp(jpgState, "dibujado") == 0 ? TFT_GREEN : TFT_LIGHTGREY);
}

void drawGps(uint32_t now) {
  char buf[40];
  const unsigned long sats = gps.satellites.isValid() ? gps.satellites.value() : 0;
  switch (currentGpsState(now)) {
    case GpsState::Waiting: drawValue("Esperando datos...", 108, TFT_YELLOW); break;
    case GpsState::NoData:  drawValue("Sin datos: revisa TX/RX", 108, TFT_RED); break;
    case GpsState::BadData: drawValue("Datos con errores", 108, TFT_ORANGE); break;
    case GpsState::UbxOnly: drawValue("UBX: pulsa OK=fabrica", 108, TFT_ORANGE); break;
    case GpsState::NoFix:
      snprintf(buf, sizeof(buf), "Sin posicion, %lu sat.", sats);
      drawValue(buf, 108, TFT_YELLOW);
      break;
    case GpsState::Fix:
      snprintf(buf, sizeof(buf), "Posicion OK, %lu sat.", sats);
      drawValue(buf, 108, TFT_GREEN);
      break;
  }
  // Frases NMEA válidas: un GPS bien conectado manda varias por segundo, con o sin posición.
  // car/s: con 0 o casi 0 no llega señal; con decenas o cientos, el GPS sí está hablando.
  snprintf(buf, sizeof(buf), "%lu frases  %lu car/s", (unsigned long)gpsSentences(),
           (unsigned long)charsPerSecond);
  drawValue(buf, 128, TFT_WHITE);
  if (gps.location.isValid()) {
    snprintf(buf, sizeof(buf), "%.6f", gps.location.lat());
    drawValue(buf, 148, TFT_WHITE);
    snprintf(buf, sizeof(buf), "%.6f", gps.location.lng());
    drawValue(buf, 168, TFT_WHITE);
  } else {
    drawValue("-", 148, TFT_DARKGREY);
    drawValue("-", 168, TFT_DARKGREY);
  }
  if (gpsBaud.locked()) {
    snprintf(buf, sizeof(buf), "%lu detectada", (unsigned long)gpsBaud.baud());
    drawValue(buf, 296, TFT_GREEN);
  } else {
    snprintf(buf, sizeof(buf), "probando %lu", (unsigned long)gpsBaud.baud());
    drawValue(buf, 296, TFT_YELLOW);
  }
}

void drawButtons() {
  char buf[40];
  snprintf(buf, sizeof(buf), "+%lu -%lu OK%lu <-%lu", (unsigned long)buttons[0].count,
           (unsigned long)buttons[1].count, (unsigned long)buttons[2].count,
           (unsigned long)buttons[3].count);
  drawValue(buf, 196, TFT_WHITE);
  drawValue(lastButton, 216, TFT_CYAN);
}

void drawBattery() {
  char buf[40];
  uint32_t mv = 0;
  for (int i = 0; i < 16; i++) mv += analogReadMilliVolts(PIN_BATTERY);  // media de 16 lecturas
  float volts = batteryVolts(mv / 16, BATTERY_DIVIDER_RATIO);
  int percent = batteryPercent(volts);
  if (percent < 0) {
    drawValue("sin medir", 244, TFT_DARKGREY);
  } else {
    snprintf(buf, sizeof(buf), "%.2f V  %d %%", volts, percent);
    drawValue(buf, 244, percent > 20 ? TFT_GREEN : TFT_ORANGE);
  }
}

void logToSerial(uint32_t now) {
  Serial.printf("t=%lus GPS car=%lu frases=%lu malas=%lu sat=%lu fix=%d lat=%.6f lon=%.6f | botones +%lu -%lu OK%lu <-%lu\n",
                (unsigned long)(now / 1000), (unsigned long)gps.charsProcessed(),
                (unsigned long)gps.passedChecksum(), (unsigned long)gps.failedChecksum(),
                (unsigned long)(gps.satellites.isValid() ? gps.satellites.value() : 0),
                gps.location.isValid(), gps.location.lat(), gps.location.lng(),
                (unsigned long)buttons[0].count, (unsigned long)buttons[1].count,
                (unsigned long)buttons[2].count, (unsigned long)buttons[3].count);
}

// El GPS habla solo en binario (UBX), normalmente porque estuvo en un dron. Esta orden borra
// lo que tenga guardado y lo deja de fábrica: 9600 baudios y frases NMEA normales.
void resetGpsToFactory(uint32_t now) {
  uint8_t frame[32];
  const size_t n = ubxFactoryReset(frame, sizeof(frame));
  Serial2.write(frame, n);
  Serial2.flush();  // que salga entera antes de cambiar de velocidad
  Serial.printf("GPS: enviada orden de fabrica a %lu baudios\n", (unsigned long)gpsBaud.baud());
  startGpsListening(now);
  Serial2.updateBaudRate(gpsBaud.baud());
}

// ---------- Arduino ----------

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Prueba GPS ESP32 ===");

  for (auto& b : buttons) pinMode(b.pin, INPUT_PULLUP);

  lcd.init();
  lcd.setRotation(PANTALLA_ROTACION);
  showColorBars();
  delay(3000);

  testSd();
  startGpsListening(millis());
  Serial2.begin(gpsBaud.baud(), SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);

  drawLayout();
  drawSdValues();
}

void loop() {
  static uint32_t lastDraw = 0;
  static uint32_t lastLog = 0;

  static uint32_t cpsLastMs = 0, cpsLastChars = 0;

  while (Serial2.available()) {
    const int c = Serial2.read();
    gps.encode((char)c);
    ubx.feed((uint8_t)c);
  }

  uint32_t now = millis();
  if (now - cpsLastMs >= 1000) {
    charsPerSecond = (gps.charsProcessed() - cpsLastChars) * 1000UL / (now - cpsLastMs);
    cpsLastChars = gps.charsProcessed();
    cpsLastMs = now;
  }
  if (gpsBaud.update(now, gpsSentences() + gpsUbxFrames())) {
    Serial2.updateBaudRate(gpsBaud.baud());
    Serial.printf("GPS: sin frases validas, pruebo %lu baudios\n", (unsigned long)gpsBaud.baud());
  }
  for (auto& b : buttons) {
    if (b.debouncer.update(digitalRead(b.pin) == LOW, now)) {
      b.count++;
      lastButton = b.name;
      Serial.printf("Boton %s\n", b.name);
      if (&b == &buttons[2] && currentGpsState(now) == GpsState::UbxOnly) resetGpsToFactory(now);
    }
  }

  if (now - lastDraw >= 500) {
    lastDraw = now;
    drawGps(now);
    drawButtons();
    drawBattery();
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu s", (unsigned long)(now / 1000));
    drawValue(buf, 272, TFT_LIGHTGREY);
  }
  if (now - lastLog >= 2000) {
    lastLog = now;
    logToSerial(now);
  }
}

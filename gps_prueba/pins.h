#pragma once
#include <stdint.h>

// Conexiones del GPS portátil (ver «Conexiones» en el documento del proyecto).

// Bus SPI compartido por pantalla y lector SD
constexpr int PIN_SPI_SCK  = 18;  // pantalla SCL · SD CLK
constexpr int PIN_SPI_MOSI = 23;  // pantalla SDA · SD MOSI
constexpr int PIN_SPI_MISO = 19;  // solo SD MISO

// Pantalla ST7789V 240x320 (GMT020-02, 7 pines, sin pin BL)
constexpr int PIN_TFT_CS  = 15;
constexpr int PIN_TFT_DC  = 2;
constexpr int PIN_TFT_RST = 4;

// Lector microSD
constexpr int PIN_SD_CS = 5;

// GPS NEO-6M (segundo puerto serie del ESP32; el ESP32 puede llevarlo a casi cualquier pin).
// Antes eran D16/D17; se cambian a D32/D13 para descartar un pin o una columna en mal estado.
// Ninguno es pin de arranque: un GPS en el 12 haría que el ESP32 no arrancase.
constexpr int PIN_GPS_RX = 32;  // aquí llega el TX del GPS
constexpr int PIN_GPS_TX = 13;  // de aquí sale al RX del GPS (34-39 no valen: solo entrada)
// Velocidades que se prueban en este orden hasta que el GPS responde (el NEO-6M viene a 9600).
constexpr uint32_t GPS_BAUDS[] = {9600, 38400, 19200, 57600, 115200, 4800, 230400};

// Botones, cada uno entre el pin y GND (pull-up interno)
constexpr int PIN_BTN_ZOOM_IN  = 25;
constexpr int PIN_BTN_ZOOM_OUT = 26;
constexpr int PIN_BTN_OK       = 27;
constexpr int PIN_BTN_BACK     = 33;

// Medida de batería: punto medio del divisor de dos resistencias iguales
constexpr int PIN_BATTERY = 35;
constexpr float BATTERY_DIVIDER_RATIO = 2.0f;

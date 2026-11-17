#pragma once
// Configuración de la pantalla para LovyanGFX. Todo se ajusta aquí, sin tocar la librería.
//
// Si en la prueba de colores algo no cuadra:
//   - Se ve en negativo (lo negro sale blanco): cambia PANTALLA_INVERTIR.
//   - El rojo sale azul y el azul rojo: cambia PANTALLA_BGR.
//   - La imagen sale boca abajo: pon PANTALLA_ROTACION a 2.
#define PANTALLA_INVERTIR true
#define PANTALLA_BGR false
#define PANTALLA_ROTACION 0

#define LGFX_USE_V1
#include <SD.h>  // antes de LovyanGFX, para poder dibujar JPG desde la SD
#include <LovyanGFX.hpp>
#include "pins.h"

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 panel_;
  lgfx::Bus_SPI bus_;

 public:
  LGFX() {
    {
      auto cfg = bus_.config();
      cfg.spi_host = SPI3_HOST;  // VSPI, el mismo bus que usa la librería SD
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = PIN_SPI_SCK;
      cfg.pin_mosi = PIN_SPI_MOSI;
      cfg.pin_miso = PIN_SPI_MISO;
      cfg.pin_dc = PIN_TFT_DC;
      bus_.config(cfg);
      panel_.setBus(&bus_);
    }
    {
      auto cfg = panel_.config();
      cfg.pin_cs = PIN_TFT_CS;
      cfg.pin_rst = PIN_TFT_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.readable = false;  // el módulo no tiene MISO
      cfg.invert = PANTALLA_INVERTIR;
      cfg.rgb_order = PANTALLA_BGR;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;  // comparte el bus con la SD
      panel_.config(cfg);
    }
    setPanel(&panel_);
  }
};

#pragma once
// Imitación de la parte de LovyanGFX que usa el sketch. Comprueba en compilación que
// ningún color se pasa como uint32_t: LovyanGFX lo leería como RGB888, no RGB565.
#include <type_traits>
#include <SD.h>
#define SPI3_HOST 2
#define SPI_DMA_CH_AUTO 3
static constexpr int TFT_BLACK = 0x0000, TFT_WHITE = 0xFFFF, TFT_RED = 0xF800, TFT_GREEN = 0x07E0,
                     TFT_BLUE = 0x001F, TFT_YELLOW = 0xFFE0, TFT_ORANGE = 0xFDA0, TFT_CYAN = 0x07FF,
                     TFT_LIGHTGREY = 0xD69A, TFT_DARKGREY = 0x7BEF;
enum class textdatum_t { top_left, middle_center };
struct IFont {};
namespace fonts { extern const IFont Font2, Font4; }
extern std::vector<std::string> g_drawn;  // textos dibujados, en orden
namespace lgfx {
template <typename T> void checkColor(T) {
  static_assert(!std::is_same<T, uint32_t>::value,
                "LovyanGFX lee los colores uint32_t como RGB888: usa uint16_t para TFT_*");
}
struct Bus_SPI {
  struct config_t { int spi_host; uint8_t spi_mode; uint32_t freq_write, freq_read; bool spi_3wire, use_lock; int dma_channel; int16_t pin_sclk, pin_mosi, pin_miso, pin_dc; };
  config_t config() const { return {}; }
  void config(const config_t&) {}
};
struct Panel_ST7789 {
  struct config_t { int16_t pin_cs, pin_rst, pin_busy; uint16_t panel_width, panel_height; int16_t offset_x, offset_y; uint8_t offset_rotation; bool readable, invert, rgb_order, dlen_16bit, bus_shared; };
  config_t config() const { return {}; }
  void config(const config_t&) {}
  void setBus(Bus_SPI*) {}
};
struct LGFX_Device {
  bool init() { return true; }
  void setRotation(uint_fast8_t) {}
  template <typename T> void fillScreen(T c) { checkColor(c); }
  template <typename T> void fillRect(int32_t, int32_t, int32_t, int32_t, T c) { checkColor(c); }
  template <typename A, typename B> void setTextColor(A fg, B bg) { checkColor(fg); checkColor(bg); }
  size_t drawString(const char* s, int32_t, int32_t) { g_drawn.push_back(s); return 0; }
  void setFont(const IFont*) {}
  void setTextDatum(textdatum_t) {}
  bool drawJpgFile(fs::FS&, const char*, int32_t = 0, int32_t = 0) { return true; }
  void setPanel(Panel_ST7789*) {}
};
}  // namespace lgfx

# Programa de prueba en protoboard — Plan de implementación

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Objetivo:** un firmware de prueba que, montado todo en la protoboard, diga en la pantalla si funcionan la pantalla, la microSD, el GPS, los 4 botones y la medida de batería.

**Arquitectura:** proyecto PlatformIO `gps-prueba/` con dos entornos: `esp32dev` (el firmware) y `native` (tests de la lógica en el PC). La lógica que no toca hardware (batería, antirrebote de botones, diagnóstico del GPS) vive en `lib/logic/` y se prueba en `native`; `src/main.cpp` solo lee el hardware y pinta resultados.

**Tecnología:** PlatformIO, plataforma `espressif32` oficial (Arduino core 2.x), TFT_eSPI (bodmer), TJpg_Decoder (bodmer), TinyGPSPlus (mikalhart), SD de Arduino, Unity para los tests.

**Spec:** documento «GPS portátil offline con ESP32 — diseño del proyecto» (secciones Conexiones, Alimentación y Firmware y uso).

## Restricciones globales

- Placa: ESP32 DevKit 30 pines, WROOM-32, sin PSRAM → `board = esp32dev`.
- Pantalla ST7789V 240×320, SPI, sin pin BL: SCL D18, SDA D23, CS D15, DC D2, RST D4, MISO compartido D19.
- Lector SD: CS D5, CLK D18, MOSI D23, MISO D19 (bus compartido con la pantalla), 3,3 V.
- GPS NEO-6M a 9600 baudios: TX del GPS → D16 (RX2), RX del GPS → D17 (TX2).
- Botones a GND con pull-up interno: zoom + D25, zoom − D26, OK D27, atrás D33.
- Batería: divisor de dos resistencias iguales → D35; relación 2,0.
- Textos en pantalla en español.

## Foco de revisión

1. GPS con TX/RX cruzados o sin alimentar: tras 5 s sin caracteres la pantalla debe decir «Sin datos del GPS: revisa TX/RX», no «esperando» para siempre. → test `test_gps_sin_datos_tras_5s` (Tarea 1).
2. Divisor de batería sin montar (pin a ~0 V): mostrar «sin medir», nunca «0 %». → test `test_bateria_sin_divisor` (Tarea 1).
3. Botón con rebotes o mantenido: un solo evento por pulsación, sin repetición. → tests `test_boton_rebote_no_cuenta` y `test_boton_mantenido_un_evento` (Tarea 1).
4. Sin tarjeta o tarjeta no FAT32: «SD no detectada» y el resto de pruebas siguen. → comprobación en placa (Tarea 2, paso 4).
5. Clones de ST7789 con colores invertidos o rojo/azul cambiados: barras de color rotuladas para que se vea a simple vista. → comprobación en placa (Tarea 2, paso 4).

---

### Task 1: lógica pura con tests

**Archivos:**
- Crear: `gps-prueba/platformio.ini` (solo el entorno `native` en esta tarea)
- Crear: `gps-prueba/lib/logic/src/battery.h`, `battery.cpp`
- Crear: `gps-prueba/lib/logic/src/debouncer.h`, `debouncer.cpp`
- Crear: `gps-prueba/lib/logic/src/gpsdiag.h`, `gpsdiag.cpp`
- Test: `gps-prueba/test/test_logic/test_main.cpp`

**Interfaces:**
- Produce:
  - `float batteryVolts(uint32_t pinMillivolts, float dividerRatio);` → V de batería.
  - `int batteryPercent(float volts);` → 0–100, o −1 si `volts < 2.5` (sin medir).
  - `class Debouncer { explicit Debouncer(uint32_t debounceMs = 30); bool update(bool rawPressed, uint32_t nowMs); bool isPressed() const; };` → `update` devuelve `true` solo en el flanco de pulsación ya estable.
  - `enum class GpsState { Waiting, NoData, NoFix, Fix };` y `GpsState gpsState(uint32_t charsProcessed, bool locationValid, uint32_t msSinceBoot);`

- [ ] **Paso 1: escribir los tests que fallan**

```cpp
void test_bateria_voltios() { TEST_ASSERT_FLOAT_WITHIN(0.001, 4.0, batteryVolts(2000, 2.0)); }
void test_bateria_llena() { TEST_ASSERT_EQUAL(100, batteryPercent(4.20)); }
void test_bateria_por_encima() { TEST_ASSERT_EQUAL(100, batteryPercent(4.35)); }
void test_bateria_vacia() { TEST_ASSERT_EQUAL(0, batteryPercent(3.30)); }
void test_bateria_media() { TEST_ASSERT_EQUAL(40, batteryPercent(3.75)); }
void test_bateria_interpolada() { TEST_ASSERT_EQUAL(45, batteryPercent(3.775)); }
void test_bateria_sin_divisor() { TEST_ASSERT_EQUAL(-1, batteryPercent(1.0)); }

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
  d.update(false, 2010); d.update(false, 2040);
  d.update(true, 2050);
  TEST_ASSERT_TRUE(d.update(true, 2080));
}

void test_gps_esperando() { TEST_ASSERT_EQUAL(GpsState::Waiting, gpsState(0, false, 2000)); }
void test_gps_sin_datos_tras_5s() { TEST_ASSERT_EQUAL(GpsState::NoData, gpsState(0, false, 5000)); }
void test_gps_sin_fix() { TEST_ASSERT_EQUAL(GpsState::NoFix, gpsState(1200, false, 60000)); }
void test_gps_fix() { TEST_ASSERT_EQUAL(GpsState::Fix, gpsState(1200, true, 60000)); }
```

- [ ] **Paso 2: comprobar que fallan**

Run: `cd gps-prueba && pio test -e native`
Esperado: FALLA al compilar (no existen `battery.h`, `debouncer.h`, `gpsdiag.h`).

- [ ] **Paso 3: implementar las tres piezas**

`batteryPercent` interpola linealmente en esta tabla LiPo (V → %): 4,20→100 · 4,10→90 · 4,00→80 · 3,90→65 · 3,80→50 · 3,75→40 · 3,70→30 · 3,65→20 · 3,60→10 · 3,50→5 · 3,30→0; por encima de 4,20 → 100, entre 2,5 y 3,30 → 0, por debajo de 2,5 → −1. `gpsState`: `locationValid` → Fix; si no, `charsProcessed > 0` → NoFix; si no, `msSinceBoot >= 5000` → NoData; si no, Waiting.

- [ ] **Paso 4: comprobar que pasan**

Run: `pio test -e native`
Esperado: 14 tests, 0 fallos.

- [ ] **Paso 5: commit**

```bash
git add gps-prueba && git commit -m "feat(prueba): lógica de batería, botones y diagnóstico GPS con tests"
```

### Task 2: firmware de prueba en el ESP32

**Archivos:**
- Modificar: `gps-prueba/platformio.ini` (añadir entorno `esp32dev` con librerías y la configuración de TFT_eSPI por `build_flags`)
- Crear: `gps-prueba/include/pins.h` (los pines de Restricciones globales como `constexpr uint8_t`)
- Crear: `gps-prueba/src/main.cpp`
- Crear: `gps-prueba/tools/make_test_jpg.py` (genera `prueba.jpg`, 240×160, con barras de color rotuladas)
- Crear: `gps-prueba/README.md` (cómo flashear y qué debe verse)

**Interfaces:**
- Consume: `batteryVolts`, `batteryPercent`, `Debouncer`, `GpsState`/`gpsState` de la Tarea 1.

- [ ] **Paso 1: configurar TFT_eSPI y compilar el esqueleto**

`build_flags` del entorno `esp32dev`: `USER_SETUP_LOADED=1`, `ST7789_DRIVER=1`, `TFT_WIDTH=240`, `TFT_HEIGHT=320`, `TFT_MOSI=23`, `TFT_SCLK=18`, `TFT_MISO=19`, `TFT_CS=15`, `TFT_DC=2`, `TFT_RST=4`, `LOAD_GLCD=1`, `LOAD_FONT2=1`, `LOAD_FONT4=1`, `SPI_FREQUENCY=40000000`, `SPI_READ_FREQUENCY=20000000`. Librerías: `bodmer/TFT_eSPI`, `bodmer/TJpg_Decoder`, `mikalhart/TinyGPSPlus`. `monitor_speed = 115200`.

Run: `pio run -e esp32dev`
Esperado: `SUCCESS`.

- [ ] **Paso 2: escribir `main.cpp`**

Arranque: pantalla en vertical, barras de color rotuladas (ROJO, VERDE, AZUL, BLANCO, NEGRO) 3 s; `SD.begin(5, SPI, 4000000)`, escribir y releer `/prueba.txt`, y si existe `/prueba.jpg` dibujarlo con TJpg_Decoder; `Serial2.begin(9600, SERIAL_8N1, 16, 17)`. Bucle: alimentar TinyGPSPlus, pasar los 4 botones por `Debouncer`, leer `analogReadMilliVolts(35)` y redibujar cada 500 ms un panel con: SD (OK / no detectada / escritura falla), GPS (texto según `GpsState`, caracteres, satélites, lat/lon), botones (nombre del último pulsado y contador de cada uno) y batería (V y %, o «sin medir»). Todo también por `Serial` a 115200.

- [ ] **Paso 3: compilar y pasar los tests**

Run: `pio run -e esp32dev && pio test -e native`
Esperado: `SUCCESS` y 14 tests sin fallos.

- [ ] **Paso 4: comprobación en placa (la hace mupsim)**

Con todo en la protoboard y el interruptor apagado (alimentado por USB):
- Barras de color: el rótulo coincide con el color. Si no, cambiar `TFT_RGB_ORDER` o `TFT_INVERSION_ON`.
- Sin tarjeta: «SD no detectada» y el resto sigue. Con tarjeta FAT32 y `prueba.jpg`: se ve la imagen.
- GPS dentro de casa: el contador de caracteres sube; si se queda en 0, aparece «revisa TX/RX». Junto a la ventana o fuera: satélites y posición.
- Cada botón suma 1 por pulsación, también si se mantiene.
- Batería: con el divisor montado marca V y %; sin él, «sin medir».

- [ ] **Paso 5: commit**

```bash
git add gps-prueba && git commit -m "feat(prueba): firmware de prueba de hardware en protoboard"
```

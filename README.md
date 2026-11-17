# GPS portátil ESP32 — programa de prueba

Comprueba en la protoboard que funcionan la pantalla, la microSD, el GPS, los 4 botones y la medida de batería. Todo sale en la pantalla y también en el Monitor Serie (115200 baudios).

## 1. Prepara el IDE de Arduino (una sola vez)

1. **Placas ESP32:** Herramientas → Placa → Gestor de placas → busca **esp32** (de Espressif Systems) → Instalar.
2. **Librerías:** Herramientas → Gestionar librerías → instala **LovyanGFX** (de lovyan03) y **TinyGPSPlus** (de Mikal Hart).
3. **Driver USB:** tu ESP32 lleva el chip CH340C. Si Windows no lo reconoce al enchufarlo, instala el driver CH340.

## 2. Monta la protoboard

Alimenta solo por el USB del ordenador. La batería, el TP4056 y el MT3608 se quedan fuera en esta prueba.

| Pieza | Pin de la pieza | Pin del ESP32 |
| --- | --- | --- |
| Pantalla | VCC · GND | 3V3 · GND |
| Pantalla | SCL · SDA | D18 · D23 |
| Pantalla | RST · DC · CS | D4 · D2 · D15 |
| Lector SD | 3v3 · GND | 3V3 · GND |
| Lector SD | CLK · MOSI | D18 · D23 (los mismos que la pantalla) |
| Lector SD | MISO · CS | D19 · D5 |
| GPS | VCC · GND | 3V3 · GND |
| GPS | TX · RX | D16 · D17 |
| Botón zoom + | una pata a GND | D25 |
| Botón zoom − | una pata a GND | D26 |
| Botón OK | una pata a GND | D27 |
| Botón atrás | una pata a GND | D33 |
| Batería (opcional) | punto medio de 2 resistencias iguales | D35 |

Los pulsadores de 12 × 12 tienen 4 patas unidas de dos en dos. Usa dos patas en **diagonal** y acertarás seguro.

## 3. Prepara la microSD (opcional)

Formatéala en **FAT32** (32 GB o menos) y copia `prueba.jpg` en la raíz.

## 4. Carga el programa

1. Abre `gps_prueba/gps_prueba.ino`. Los demás archivos de la carpeta se abren solos en pestañas.
2. Placa: **ESP32 Dev Module**. Puerto: el COM que aparece al enchufar el ESP32.
3. Pulsa **Subir**. Si se queda en «Connecting…», mantén pulsado el botón **BOOT** de la placa hasta que empiece a cargar.
4. Abre el Monitor Serie a 115200.

## 5. Qué tienes que ver

| Paso | Bien | Si no |
| --- | --- | --- |
| Barras de color (3 s) | ROJO, VERDE, AZUL, BLANCO y NEGRO de arriba abajo, con «^ ARRIBA» arriba | En `lgfx_config.h`: si sale en negativo, cambia `PANTALLA_INVERTIR`; si rojo y azul van cambiados, cambia `PANTALLA_BGR`; si sale boca abajo, pon `PANTALLA_ROTACION` a 2 |
| SD | «OK» | «No detectada»: revisa los cables y que sea FAT32. El resto de pruebas sigue igual |
| JPG (si copiaste `prueba.jpg`) | Ves la imagen 3 s y luego «dibujado» | «no se pudo dibujar»: vuelve a generar la imagen |
| GPS dentro de casa | Los caracteres («car.») suben | Si a los 5 s sale «Sin datos: revisa TX/RX», cruza TX y RX |
| GPS junto a una ventana o fuera | Suben los satélites y sale «Posicion OK» con lat y lon | La primera vez puede tardar varios minutos a cielo abierto |
| Botones | Cada pulsación suma 1, aunque lo mantengas | Si no suma, prueba otras dos patas del pulsador |
| Batería | Con el divisor: voltios y %. Sin él: «sin medir» | — |

## Para desarrolladores: tests de la lógica

```bash
git clone --depth 1 https://github.com/ThrowTheSwitch/Unity.git ../vendor/Unity   # una vez
make -C tests
```

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
| Resistencia de abajo (siempre) | entre D35 y GND | D35 |
| Resistencia de arriba (solo con batería) | entre D35 y el + de la batería | D35 |

Los pulsadores de 12 × 12 tienen 4 patas unidas de dos en dos. Usa dos patas en **diagonal** y acertarás seguro.

Pon **siempre la resistencia de abajo** (D35 a GND), aunque no midas la batería: así el pin no queda al aire y da lecturas falsas. Como en esta prueba la batería va fuera, lo correcto es que salga «sin medir».

## 3. Prepara la microSD (opcional)

Formatéala en **FAT32** (32 GB o menos) y copia en la raíz el archivo `sd/prueba.jpg` que viene en esta carpeta. No uses otra foto: muchas son JPEG progresivos y el ESP32 no los dibuja.

## 4. Carga el programa

1. Abre `gps_prueba/gps_prueba.ino`. Los demás archivos de la carpeta se abren solos en pestañas.
2. Placa: **ESP32 Dev Module**. Puerto: el COM que aparece al enchufar el ESP32.
3. Pulsa **Subir**. Si se queda en «Connecting…», mantén pulsado el botón **BOOT** de la placa hasta que empiece a cargar. Si falla con «Wrong boot mode (0x13)», desconecta el cable DC (D2) mientras subes y vuelve a ponerlo después. Que el LED azul de la placa parpadee es normal: comparte pin con DC.
4. Abre el Monitor Serie a 115200.

## 5. Qué tienes que ver

| Paso | Bien | Si no |
| --- | --- | --- |
| Barras de color (3 s) | ROJO, VERDE, AZUL, BLANCO y NEGRO de arriba abajo, con «^ ARRIBA» arriba | En `lgfx_config.h`: si sale en negativo, cambia `PANTALLA_INVERTIR`; si rojo y azul van cambiados, cambia `PANTALLA_BGR`; si sale boca abajo, pon `PANTALLA_ROTACION` a 2 |
| Pantalla en blanco o con rayas | — | Revisa los cables de la pantalla. Si siguen bien, en `lgfx_config.h` baja `cfg.freq_write` de 40000000 a 20000000: en protoboard los cables largos a veces no aguantan tanta velocidad |
| SD | «OK» | «No detectada»: revisa los cables y que sea FAT32. El resto de pruebas sigue igual |
| JPG (si copiaste `prueba.jpg`) | Ves la imagen 3 s y luego «dibujado» | «no se pudo dibujar»: vuelve a copiar `sd/prueba.jpg` |
| GPS dentro de casa | «Sin posicion todavia» y las frases suben varias por segundo, aunque no haya satélites | «Sin datos: revisa TX/RX»: el GPS no manda nada. 1) Mide con el polímetro entre VCC y GND del GPS: tiene que dar unos 3,3 V. 2) Comprueba que el TX del GPS va a D16 y su RX a D17; si dudas, intercambia esos dos cables (no se rompe nada). «Datos raros: baudios?»: el GPS habla a otra velocidad (pasará con el M10 hasta adaptar el programa) |
| GPS junto a una ventana o fuera | Suben los satélites y sale «Posicion OK» con lat y lon. El LED del GPS empieza a parpadear una vez por segundo | La primera vez puede tardar bastantes minutos a cielo abierto. El LED del NEO-6M **no indica que tenga corriente**: solo parpadea cuando ya tiene posición, así que apagado es normal mientras busca |
| Botones | Cada pulsación suma 1, aunque lo mantengas | Si no suma, prueba otras dos patas del pulsador |
| Batería | Sin batería (esta prueba): «sin medir». Con batería y las dos resistencias: voltios y % | Si sin batería marca voltios, falta la resistencia de abajo |

## Para desarrolladores

Tests en el PC (lógica, sketch completo con imitaciones de las librerías, e imagen de la SD):

```bash
git clone --depth 1 https://github.com/ThrowTheSwitch/Unity.git ../vendor/Unity   # una vez
pip install "pillow>=10.1"                                                       # una vez
make -C tests
```

Para volver a generar la imagen de prueba: `python tools/make_test_jpg.py sd`.

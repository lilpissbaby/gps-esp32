#pragma once
#include <stdint.h>

// Estado del GPS para la pantalla de prueba. Solo cuentan como datos las frases NMEA
// completas y con la suma de control bien: un cable suelto o tocado mete caracteres de
// ruido sueltos, y eso no es el GPS hablando.
//  Waiting: aún no ha llegado ninguna frase, pero llevamos menos de 5 s.
//  NoData:  5 s o más sin frases y casi sin caracteres -> alimentación o TX/RX.
//  BadData: llegan muchos caracteres pero ninguna frase válida -> mal contacto (pines del GPS
//           sin soldar, GND) u otra velocidad (el programa prueba velocidades solo).
//  NoFix:   llegan frases pero aún no hay posición.
//  Fix:     hay posición válida.
enum class GpsState { Waiting, NoData, BadData, NoFix, Fix };

// A partir de cuántos caracteres sin frases válidas lo consideramos basura y no ruido.
constexpr uint32_t GPS_GARBAGE_CHARS = 200;

GpsState gpsState(uint32_t charsProcessed, uint32_t validSentences, bool locationValid,
                  uint32_t msListening);

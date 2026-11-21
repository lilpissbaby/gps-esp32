#pragma once
#include <stdint.h>

// Estado del GPS para la pantalla de prueba. Solo cuentan como datos las frases NMEA (o
// tramas UBX) completas y con la suma de control bien, y la "señal" se mide en caracteres
// por segundo: un cable suelto o tocado mete algún carácter suelto, y eso no es el GPS.
//  Waiting: aún no ha llegado nada válido, pero llevamos menos de 5 s.
//  NoData:  no llega señal (casi 0 car/s) -> alimentación o TX/RX.
//  BadData: llega señal (muchos car/s) pero nada válido -> mal contacto u otra velocidad.
//  UbxOnly: llegan tramas UBX y ninguna frase NMEA -> GPS configurado solo en binario.
//  NoFix:   llegan frases pero aún no hay posición.
//  Fix:     hay posición válida.
enum class GpsState { Waiting, NoData, BadData, UbxOnly, NoFix, Fix };

// A partir de cuántos caracteres por segundo consideramos que hay señal y no ruido.
constexpr uint32_t GPS_SIGNAL_CPS = 10;

GpsState gpsState(uint32_t charsPerSecond, uint32_t validSentences, uint32_t ubxFrames,
                  bool locationValid, uint32_t msListening);

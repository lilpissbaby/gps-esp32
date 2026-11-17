#pragma once
#include <stdint.h>

// Estado del GPS para la pantalla de prueba.
//  Waiting: aún no han llegado datos, pero llevamos menos de 5 s.
//  NoData:  5 s o más sin un solo carácter -> cableado TX/RX o alimentación.
//  NoFix:   llegan datos pero aún no hay posición.
//  Fix:     hay posición válida.
enum class GpsState { Waiting, NoData, NoFix, Fix };

GpsState gpsState(uint32_t charsProcessed, bool locationValid, uint32_t msSinceBoot);

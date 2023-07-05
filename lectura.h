#ifndef LECTURA_H
#define LECTURA_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "poligono.h"

#ifndef COLOR_T
#define COLOR_T
typedef enum {
	AZUL = 0,
	NARANJA = 1,
	VERDE = 2,
	GRIS = 3,
	AMARILLO = 4,
	BLANCO = 5,
}color_t;
#endif //COLOR_T

typedef enum {
	INMOVIL = 0,
	CIRCULAR = 1,
	HORIZONTAL = 2,
}movimiento_t;

typedef enum {
	CIRCULO = 0,
	RECTANGULO = 1,
	POLIGONO = 2,
}geometria_t;

extern const char *colores[];

bool leer_encabezado(FILE *f, color_t *color, movimiento_t *movimiento, geometria_t *geometria);

bool leer_movimiento_inmovil(FILE *f, int16_t parametros[], size_t *n_parametros);

bool leer_movimiento_circular(FILE *f, int16_t parametros[], size_t *n_parametros);

bool leer_movimiento_horizontal(FILE *f, int16_t parametros[], size_t *n_parametros);

typedef bool (*leer_movimiento_t)(FILE *f, int16_t parametros[], size_t *n_parametros);

bool leer_movimiento(FILE *f, movimiento_t movimiento, int16_t parametros[], size_t *n_parametros);

poligono_t *leer_geometria_circulo(FILE *f);

poligono_t *leer_geometria_rectangulo(FILE *f);

poligono_t *leer_geometria_poligono(FILE *f);

typedef poligono_t *(*leer_geometria_t)(FILE *f);

poligono_t *leer_geometria(FILE*f, geometria_t geometria);

#endif // NIVELES_H
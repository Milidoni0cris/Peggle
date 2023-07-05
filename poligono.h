#ifndef POLIGONO_H
#define POLIGONO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif //PI

typedef struct poligono poligono_t;

#ifndef COLOR_T
#define COLOR_T
typedef enum{
    AZUL = 0,
    NARANJA = 1,
    VERDE = 2,
    GRIS = 3,
    AMARILLO = 4,
    BLANCO = 5,
}
color_t;
#endif //COLOR_T

extern const uint8_t colores_exa[6][3];

float producto_interno(float ax, float ay, float bx, float by);

poligono_t *poligono_crear(float vertices[][2], size_t n);

void poligono_destruir(poligono_t *poligono);

size_t poligono_cantidad_vertices(const poligono_t *poligono);

bool poligono_obtener_vertice(const poligono_t *poligono, size_t pos, float *x, float *y);

poligono_t *poligono_clonar(const poligono_t *poligono);

bool poligono_agregar_vertice(poligono_t *poligono, float x, float y);

void poligono_rotar(poligono_t *p, double rad);

void poligono_trasladar(poligono_t *p, float dx, float dy);

void poligono_rotar_centrado(poligono_t *p, float cx, float cy, double rad);

double poligono_distancia(const poligono_t *p, float xp, float yp, float *nor_x, float *nor_y);

poligono_t *poligono_crear_circulo(float radio, float cx, float cy);

void poligono_imprimir_pruebas(poligono_t *p);

void poligono_dibujar_SDL(poligono_t *p, color_t color,SDL_Renderer * renderer);

#endif //POLIGONO_H
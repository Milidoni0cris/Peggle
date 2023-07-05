#ifndef OBSTACULO_T
#define OBSTACULO_T

#include <stdio.h>
#include <stdbool.h>
#include "lectura.h"
#include <SDL2/SDL.h>

typedef struct obstaculo obstaculo_t;

obstaculo_t *obstaculo_crear();

void obstaculo_destruir(obstaculo_t *o);

bool obstaculo_leer(obstaculo_t *o,FILE *f);

void obstaculo_dibujar_SDL(obstaculo_t *o, SDL_Renderer *renderer);

void obstaculo_cambiar_color(obstaculo_t *o,color_t nuevo_color);

void obstaculo_prueba(obstaculo_t *o);

double obstaculo_distancia(obstaculo_t *o, float xp, float yp, float *nor_x, float *nor_y);

geometria_t obstaculo_geometria(obstaculo_t *o);

color_t obstaculo_color(obstaculo_t *o);

movimiento_t obstaculo_movimiento(obstaculo_t *o);

size_t obstaculo_n_parametros(obstaculo_t *o);

obstaculo_t *obstaculo_crear_desde_archivo(FILE* f);

void obstaculo_parametros(obstaculo_t *o, int16_t vector[3], size_t *n_parametros);

void obstaculo_trasladar(obstaculo_t *o, float dx,float dy);

void obstaculo_rotar_centrado(obstaculo_t *o,float cx,float cy, double angulo);

void obstaculo_actualizar_parametro_1(obstaculo_t *o, int16_t n);

void obstaculo_actualizar_parametro_2(obstaculo_t *o, int16_t n);

void obstaculo_actualizar_parametro_3(obstaculo_t *o, int16_t n);

void obstaculo_ejecutar_movimiento_horizontal(obstaculo_t *o, float dt);

void obstaculo_ejecutar_movimiento_inmovil(obstaculo_t *o, float dt);

void obstaculo_ejecutar_movimiento_circular(obstaculo_t *o, float dt);

typedef void (*ejecutar_movimiento_t)(obstaculo_t *o, float dt);

void obstaculo_ejecutar_movimiento(obstaculo_t *o,float dt);

#endif //OBSTACULO_T
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "poligono.h"
#include "lectura.h"
//#include <SDL2/SDL.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif //PI

struct poligono{
	float (*vertices)[2];
	size_t n;
};

const uint8_t colores_exa[6][3]={
	[AZUL]={0x00,0x00,0xFF},
	[NARANJA]={0xFF,0x80,0x00},
	[VERDE]={0x00,0xFF,0x00},
	[GRIS] = {0x9B,0x9B,0x9B},
	[AMARILLO] = {0xff,0xff,0x00},
	[BLANCO] = {0xff,0xff,0xff},
};

float producto_interno(float ax, float ay, float bx, float by){ //cambiarla de lugar
    return ax * bx + ay * by;
}

static double distancia(float ax, float ay, float bx, float by){
    return sqrt((ax - bx) * (ax - bx) + (ay - by) * (ay - by));
}


poligono_t *poligono_crear(float vertices[][2], size_t n){
	poligono_t *p=malloc(sizeof(poligono_t));
	
	if(p==NULL){
		return NULL;
	}
	if(vertices==NULL){
		p->n=0;
		p->vertices=NULL;
		return p;
	}
	p->vertices=malloc(n*sizeof(float[2]));
	if(p->vertices==NULL){
		free(p);
		return NULL;
	}
	for(size_t i=0;i<n;i++){
		(p->vertices)[i][0]=vertices[i][0];
		(p->vertices)[i][1]=vertices[i][1];
	}
	p->n=n;
	return p;
}

void poligono_destruir(poligono_t *poligono){
	free(poligono->vertices);
	free(poligono);
}

//Getters
size_t poligono_cantidad_vertices(const poligono_t *poligono){
	return poligono->n;
}

bool poligono_obtener_vertice(const poligono_t *poligono, size_t pos, float *x, float *y){
	if(poligono->n<=pos) return false;
	*x=poligono->vertices[pos][0];
	*y=poligono->vertices[pos][1];
	return true;
}

poligono_t *poligono_clonar(const poligono_t *poligono){
	poligono_t *clon=poligono_crear(poligono->vertices,poligono->n);
	return clon;
}

bool poligono_agregar_vertice(poligono_t *poligono, float x, float y){
	float (*aux)[2]=realloc(poligono->vertices,(poligono->n+1)*sizeof(float[2]));
	if(aux==NULL){
		return false;
	}
	poligono->vertices=aux;
	(poligono->vertices)[poligono->n][0]=x;
	(poligono->vertices)[poligono->n][1]=y;
	poligono->n++;
	return true;
}

void poligono_rotar(poligono_t *p, double rad){
	for(size_t i=0;i<(p->n);i++){
			float x=(p->vertices)[i][0];
			float y=(p->vertices)[i][1];
			(p->vertices)[i][0]=x*cos(rad)-y*sin(rad);
			(p->vertices)[i][1]=x*sin(rad)+y*cos(rad);
		}
}

void poligono_trasladar(poligono_t *p, float dx, float dy){
	for(size_t i=0;i<(p->n);i++){
		(p->vertices)[i][0]+=dx;
		(p->vertices)[i][1]+=dy;
	}
}

void poligono_rotar_centrado(poligono_t *p,float cx,float cy,double rad){
	poligono_trasladar(p,-cx,-cy);
	poligono_rotar(p,rad);
	poligono_trasladar(p,cx,cy);
}

static void punto_mas_cercano(float x0, float y0, float x1, float y1, float xp, float yp, float *x, float *y){
	float ax = xp - x0;
	float ay = yp - y0;
	float bx = x1 - x0;
	float by = y1 - y0;

	float alfa = producto_interno(ax, ay, bx, by) / producto_interno(bx, by, bx, by);

	if (alfa <= 0){
		*x = x0;
		*y = y0;
	}
	else if (alfa >= 1){
		*x = x1;
		*y = y1;
	}
	else{
		*x = alfa * bx + x0;
		*y = alfa * by + y0;
	}
}

double poligono_distancia(const poligono_t *p, float xp, float yp, float *nor_x, float *nor_y){
	double d = 1 / 0.0;
	size_t idx = 0;

	for (size_t i = 0; i < p->n; i++){
		float xi, yi;
		punto_mas_cercano(p->vertices[i][0], p->vertices[i][1], p->vertices[(i + 1) % p->n][0], p->vertices[(i + 1) % p->n][1], xp, yp, &xi, &yi);
		double di = distancia(xp, yp, xi, yi);

		if (di < d){
			d = di;
			idx = i;
		}
	}
	float nx = p->vertices[(idx + 1) % p->n][1] - p->vertices[idx][1];
	float ny = p->vertices[idx][0] - p->vertices[(idx + 1) % p->n][0];
	float dn = distancia(nx, ny, 0, 0);

	nx /= dn;
	ny /= dn;

	*nor_x = nx;
	*nor_y = ny;

	return d;
}

poligono_t *poligono_crear_circulo(float radio, float cx, float cy){
	poligono_t *p=poligono_crear(NULL,0);
	if(p==NULL){
		return NULL;
	}

	for(size_t i=0;i<20;i++){
		poligono_agregar_vertice(p,cx+radio,cy);
		poligono_rotar_centrado(p,cx,cy,-PI/10);
	}
	return p;
}

void poligono_dibujar_SDL(poligono_t *p,color_t color, SDL_Renderer * renderer){
 	SDL_SetRenderDrawColor(renderer, colores_exa[color][0], colores_exa[color][1], colores_exa[color][2], 0x00);
	size_t i = 0;
	for (; i < (p->n) - 1; i++){
		SDL_RenderDrawLine(renderer, p->vertices[i][0], p->vertices[i][1], p->vertices[i + 1][0], p->vertices[i+1][1]);
	}
	SDL_RenderDrawLine(renderer, p->vertices[i][0], p->vertices[i][1], p->vertices[0][0], p->vertices[0][1]);
}

void poligono_imprimir_pruebas(poligono_t *p){
	for(size_t i=0;i< p->n;i++){
		printf("(%.0f,%.0f)",p->vertices[i][0],p->vertices[i][1]);
		putchar('\t');
	}
	putchar('\n');
}



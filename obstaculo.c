#include "obstaculo.h"
#include "poligono.h"
#include "lectura.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

struct obstaculo{
    geometria_t g;
    color_t c;
    movimiento_t m;
    poligono_t *p;
    int16_t parametros[3];
    size_t n_parametros;
};


obstaculo_t *obstaculo_crear(){
	obstaculo_t *o=malloc(sizeof(obstaculo_t));
	if(o==NULL){
		return NULL;
	}
	o->p=NULL;
	return o;
}

void obstaculo_destruir(obstaculo_t *o){
	if(o->p!=NULL){
		poligono_destruir(o->p);
	}
	free(o);
}

bool obstaculo_leer(obstaculo_t *o,FILE *f){
	if(!leer_encabezado(f,&(o->c),&(o->m),&(o->g))){
		return false;
	}
	if(!leer_movimiento(f,o->m,o->parametros,&(o->n_parametros))){
		return false;
	}
	o->p=leer_geometria(f,o->g);
	if(o->p==NULL){
		return false;
	}
	return true;
}

obstaculo_t *obstaculo_crear_desde_archivo(FILE *f){
	obstaculo_t *o = malloc(sizeof(obstaculo_t));
	if (o == NULL){
		return NULL;
	}
	if (!leer_encabezado(f, &(o->c), &(o->m), &(o->g))){
		free(o);
		return NULL;
	}
	if (!leer_movimiento(f, o->m, o->parametros, &(o->n_parametros))){
		free(o);
		return NULL;
	}
	o->p = leer_geometria(f, o->g);
	if (o->p == NULL){
		free(o);
		return NULL;
	}
	return o;
}

void obstaculo_cambiar_color(obstaculo_t *o, color_t nuevo_color){
	o->c=nuevo_color;
}

void obstaculo_dibujar_SDL(obstaculo_t * o, SDL_Renderer * renderer){
	poligono_dibujar_SDL(o->p,o->c,renderer);
}
//double poligono_distancia(const poligono_t *p, float xp, float yp, float *nor_x, float *nor_y)
double obstaculo_distancia(obstaculo_t *o, float xp, float yp, float *nor_x, float *nor_y){
	return poligono_distancia(o->p, xp, yp, nor_x, nor_y);
}

geometria_t obstaculo_geometria (obstaculo_t *o){
	return o->g;
}

color_t obstaculo_color(obstaculo_t* o){
	return o->c;
}

movimiento_t obstaculo_movimiento(obstaculo_t *o){
	return o->m;
}

size_t obstaculo_n_parametros(obstaculo_t *o){
	return o->n_parametros;
}

void obstaculo_parametros(obstaculo_t *o,int16_t vector[3],size_t *n_parametros){
	vector[0] = o->parametros[0];
	vector[1] = o->parametros[1];
	vector[2] = o->parametros[2];
	*n_parametros=o->n_parametros;
}

void obstaculo_rotar_centrado(obstaculo_t *o,float cx,float cy,double angulo){
	poligono_rotar_centrado(o->p,cx,cy,angulo);
}

void obstaculo_trasladar(obstaculo_t *o,float dx,float dy){
	poligono_trasladar(o->p,dx,dy);
}

void obstaculo_actualizar_parametro_1(obstaculo_t *o,int16_t n){
	o->parametros[0] = n;
}

void obstaculo_actualizar_parametro_2(obstaculo_t *o, int16_t n){
	o->parametros[1] = n;
}

void obstaculo_actualizar_parametro_3(obstaculo_t *o, int16_t n){
	o->parametros[2] = n;		
}

void obstaculo_ejecutar_movimiento_horizontal(obstaculo_t *o,float dt){
	size_t n_parametros;
	int16_t parametros[3];
	obstaculo_parametros(o, parametros, &n_parametros);

	if (parametros[1] > parametros[0]){
		parametros[2] = -parametros[2];
		obstaculo_actualizar_parametro_3(o, parametros[2]);
	}
	if (parametros[1] < 0){
		parametros[2] = -parametros[2];
		obstaculo_actualizar_parametro_3(o, parametros[2]);
	}
	obstaculo_trasladar(o, parametros[2] * dt, 0);
	if (parametros[2] > 0){
		parametros[1]++;
	}
	else{
		parametros[1]--;
	}
	obstaculo_actualizar_parametro_2(o, parametros[1]);
}

void obstaculo_ejecutar_movimiento_inmovil(obstaculo_t *o,float dt){
	return;
}

void obstaculo_ejecutar_movimiento_circular(obstaculo_t *o,float dt){
	size_t n_parametros;
	int16_t parametros[3];
	obstaculo_parametros(o, parametros, &n_parametros);
	obstaculo_rotar_centrado(o, parametros[0], parametros[1], parametros[2] * dt);
}

const ejecutar_movimiento_t ejecutar_movimiento_tipo[3] = {
	[INMOVIL] = obstaculo_ejecutar_movimiento_inmovil,
	[CIRCULAR] = obstaculo_ejecutar_movimiento_circular,
	[HORIZONTAL] = obstaculo_ejecutar_movimiento_horizontal,
};

void obstaculo_ejecutar_movimiento(obstaculo_t *o,float dt){
	ejecutar_movimiento_tipo[obstaculo_movimiento(o)](o,dt);
}
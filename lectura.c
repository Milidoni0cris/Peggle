#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lectura.h"

#define MASK_COLOR 0xC0
#define SHIFT_COLOR 6

#define MASK_MOVIMIENTO 0x30
#define SHIFT_MOVIMIENTO 4

#define MASK_GEOMETRIA 0x0F

const char *colores[] = {
    [0]="Azul",
    [1]="Naranja",
    [2]="Verde",
    [3]="Gris",
};

bool leer_encabezado(FILE *f, color_t *color, movimiento_t *movimiento, geometria_t *geometria){
	uint8_t dato;
	if(fread(&dato,sizeof(uint8_t),1,f)!=1) return false;
	
	*color=(dato & MASK_COLOR)>>SHIFT_COLOR;
	*movimiento=(dato & MASK_MOVIMIENTO)>>SHIFT_MOVIMIENTO;
	*geometria=(dato & MASK_GEOMETRIA);

	if(*color>3) return false;
	if(*movimiento>2) return false;
	if(*geometria>2) return false;

	return true;
}

bool leer_movimiento_inmovil(FILE *f, int16_t parametros[], size_t *n_parametros){
	*n_parametros=0;
	return true;
}

bool leer_movimiento_circular(FILE *f, int16_t parametros[], size_t *n_parametros){
	if(fread(parametros,sizeof(int16_t),3,f)!=3){
		return false;
	}
	*n_parametros=3;
	return true;
}

bool leer_movimiento_horizontal(FILE *f, int16_t parametros[], size_t *n_parametros){
	if(fread(parametros,sizeof(int16_t),3,f)!=3){
		return false;
	}
	*n_parametros=3;
	return true;
}

const leer_movimiento_t lectura_movimiento_tipo[3]={
	[INMOVIL]=leer_movimiento_inmovil,
	[CIRCULAR]=leer_movimiento_circular,
	[HORIZONTAL]=leer_movimiento_horizontal,
};


bool leer_movimiento(FILE *f, movimiento_t movimiento, int16_t parametros[], size_t *n_parametros){
	return lectura_movimiento_tipo[movimiento](f,parametros,n_parametros);
}

poligono_t *leer_geometria_circulo(FILE *f){ // 3 parámetros: x, y, radio.
	int16_t x,y,radio;

	if(fread(&x,sizeof(int16_t),1,f)!= 1);
	if(fread(&y,sizeof(int16_t),1,f) != 1);
	if(fread(&radio,sizeof(int16_t),1,f) != 1);
	
	poligono_t *p=poligono_crear_circulo(radio,x,y);
	if(p==NULL){
		return NULL;
	}

	return p;
}

poligono_t *leer_geometria_rectangulo(FILE *f){
	int16_t x,y,ancho,alto,angulo;

	if(fread(&x,sizeof(int16_t),1,f)!=1);
	if(fread(&y,sizeof(int16_t),1,f)!=1);
	if(fread(&ancho,sizeof(int16_t),1,f)!=1);
	if(fread(&alto,sizeof(int16_t),1,f)!=1);
	if(fread(&angulo,sizeof(int16_t),1,f)!=1);

	float (*vertices)[2] = malloc(4* sizeof(float[2]));
	if(vertices==NULL){
		return NULL;
	}
	vertices[0][0] = ancho/2;
	vertices[0][1] = alto/2;

	vertices[1][0] = -ancho/2; 
	vertices[1][1] = alto/2;

	vertices[2][0] = -ancho/2;
	vertices[2][1] = -alto/2;

	vertices[3][0] = ancho/2;
	vertices[3][1] = -alto/2;
	
	poligono_t *p = poligono_crear(vertices, 4);

	if(p == NULL){
		free(vertices);
		return NULL; 
	}
	free(vertices);

	poligono_rotar(p, angulo *(PI / 180));
	poligono_trasladar(p, x, y);
	//poligono_rotar_centrado(p,x,y,angulo *(180/PI));
	return p;
}


poligono_t *leer_geometria_poligono(FILE *f){
	int16_t cant_vertices;
	if (fread(&cant_vertices, sizeof(int16_t), 1, f) != 1){
		return NULL;
	}
	int16_t (*vertices)[2] = malloc(cant_vertices * sizeof(int16_t[2]));
	if (vertices == NULL){
		return false;
	}
	if (fread(vertices, sizeof(int16_t[2]), cant_vertices, f) != cant_vertices){
		free(vertices);
		return NULL;
	}
	float (*vertices2)[2] = malloc(cant_vertices * sizeof(float[2]));
	if (vertices2 == NULL){
		return false;
	}
	for(size_t i=0;i<cant_vertices;i++){
		vertices2[i][0] = vertices[i][0];
		vertices2[i][1] = vertices[i][1];
	}
	poligono_t *p = poligono_crear(vertices2, cant_vertices);
	if (p == NULL){
		free(vertices);
		free(vertices2);
		return NULL;
	}
	free(vertices);
	free(vertices2);
	return p;
}

const leer_geometria_t lectura_geometria_tipo[3]={
	[CIRCULO]=leer_geometria_circulo,
	[RECTANGULO]=leer_geometria_rectangulo,
	[POLIGONO]=leer_geometria_poligono,
};

poligono_t *leer_geometria(FILE*f, geometria_t geometria){
	return lectura_geometria_tipo[geometria](f);
}

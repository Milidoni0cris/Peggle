#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#define TTF

#include "config.h"
#include "poligono.h"
#include "lectura.h"
#include "obstaculo.h"
#include "lista.h"

#define DT (1.0 / JUEGO_FPS)

#define STR_MAX 50
#define V_MIN 0.4

#ifdef TTF
#include <SDL2/SDL_ttf.h>

//Puntaje------------------------------------------------------------
#define CANT_MULT 4

typedef struct{
    unsigned int puntos;
    unsigned char multiplicador;
    unsigned char naranjas;
}puntaje_t;

unsigned char multiplicadores[CANT_MULT][2] = {{10, 2}, {15, 3}, {19, 5}, {21, 10}};

unsigned char actualizar_multiplicador(unsigned char naranjas,unsigned char actual){
    for(size_t i=0;i< CANT_MULT;i++){
        if(naranjas==multiplicadores[i][0]){
            return multiplicadores[i][1];
        }
    }
    return actual;
}

//---------------------------------------------------------------------

double computar_velocidad(double vi, double a, double dt){
    return dt * a + vi;
}

double computar_posicion(double pi, double vi, double dt){
    return dt * vi + pi;
}

void wrapper_obstaculo_destruir(void *dato){
    obstaculo_t *o=dato;
    obstaculo_destruir(o);
}

void escribir_texto(SDL_Renderer *renderer, TTF_Font *font, const char *s, int x, int y,color_t c){
    SDL_Color color = {colores_exa[c][0], colores_exa[c][1], colores_exa[c][2]}; // EstarÃ­a bueno si la funciÃ³n recibe un enumerativo con el color, Â¿no?
    SDL_Surface *surface = TTF_RenderText_Solid(font, s, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
#endif

void reflejar(float norm_x, float norm_y, float *cx, float *cy, float *vx, float *vy) {
    float proy = producto_interno(norm_x, norm_y, *vx, *vy);

    if(proy >= 0)
        return;

    *vx -= 2 * norm_x * proy;
    *vy -= 2 * norm_y * proy;

    // Además empujamos a la bola hacia afuera para que no se pegue
    *cx += norm_x * 0.1;
    *cy += norm_y * 0.1;
}

//------------------------------------------------------------------------------------------------------//

int main(int argc, char *argv[]){
    if(argc!=2){
        fprintf(stderr,"Uso ./peggle <nivel.bin>\n");
        return 1;
    }

    FILE *archivo=fopen(argv[1],"rb");
    if(archivo == NULL){
        fprintf(stderr, "No se pudo cargar el juego :(\n");
        return 1;
    }

    int16_t cant_obstaculos;
    int16_t *cant_obstaculos_por_nivel=NULL;
    size_t cant_niveles=0;
    lista_t **niveles=NULL;

    while (fread(&cant_obstaculos, sizeof(int16_t), 1, archivo) == 1){
        cant_niveles ++;
        int16_t *aux1=realloc(cant_obstaculos_por_nivel,cant_niveles*sizeof(int16_t));
        if(aux1 == NULL){
            free(cant_obstaculos_por_nivel);
            free(niveles);
            fclose(archivo);
            fprintf(stderr,"ERROR SEGMENTATION CORE");
            return 1;
        }
        cant_obstaculos_por_nivel=aux1;
        cant_obstaculos_por_nivel[cant_niveles-1]=cant_obstaculos;

        lista_t **aux2 = realloc(niveles, cant_niveles * sizeof(lista_t *));
        if (aux2 == NULL){
            free(cant_obstaculos_por_nivel);
            free(niveles);
            fclose(archivo);
            fprintf(stderr, "ERROR SEGMENTATION CORE");
            return 1;
        }
        niveles=aux2;
        niveles[cant_niveles-1]= lista_crear();
        if (niveles[cant_niveles-1] == NULL){
            for (size_t j = 0; j < cant_niveles-1; j++){
                lista_destruir(niveles[j], wrapper_obstaculo_destruir);
            }
            free(niveles);
            free(cant_obstaculos_por_nivel);
            fclose(archivo);
            fprintf(stderr, "ERROR SEGMENTATION CORE");
            return 1;
        }
        obstaculo_t *o=NULL;
        for(size_t i=0;i<cant_obstaculos;i++){
            o=obstaculo_crear_desde_archivo(archivo);
            if(o==NULL){
                for(size_t j=0;j<cant_niveles;j++){
                    lista_destruir(niveles[j],wrapper_obstaculo_destruir);
                }
                free(niveles);
                free(cant_obstaculos_por_nivel);
                fclose(archivo);
                fprintf(stderr, "ERROR SEGMENTATION CORE");
                return 1;
            }
            lista_insertar_primero(niveles[cant_niveles-1],o);
        }
    }

    fclose(archivo);

    //Puntaje
    puntaje_t score[cant_niveles];

    SDL_Init(SDL_INIT_VIDEO);

    #ifdef TTF
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("FreeSansBold.ttf", 24);
    #endif

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Peggle by Cristian Milidoni & Nicolas Nobili");
    
    int dormir = 0;

    // BEGIN cÃ³digo del alumno
    float canon_angulo = 0; // Ãngulo del caÃ±Ã³n
    bool cayendo = false;   // Â¿Hay bola disparada?

    float cx, cy; // Centro de la bola
    float vx, vy; // Velocidad de la bola
    // END cÃ³digo del alumno

    unsigned int ticks = SDL_GetTicks();
    for(size_t j=0;j<cant_niveles;j++){
        score[j].puntos = 0;
        score[j].multiplicador = 1;
        score[j].naranjas=0;
        char cadena_puntos[STR_MAX];
        size_t vidas=10;

        while (1){
            if (SDL_PollEvent(&event)){
                if (event.type == SDL_QUIT)
                    break;

                // BEGIN cÃ³digo del alumno
                if (event.type == SDL_MOUSEBUTTONDOWN){
                    if (!cayendo)
                        cayendo = true;
                }
                else if (event.type == SDL_MOUSEMOTION){
                    canon_angulo = atan2(event.motion.x - CANON_X, event.motion.y - CANON_Y);
                    if (canon_angulo > CANON_MAX)
                        canon_angulo = CANON_MAX;
                    if (canon_angulo < -CANON_MAX)
                        canon_angulo = -CANON_MAX;
                }
                // END cÃ³digo del alumno

                continue;
            }
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);

            // BEGIN cÃ³digo del alumno
            #ifdef TTF
            escribir_texto(renderer, font, "Peggle Version Algoritmos 9511", 100, 20, BLANCO);
            sprintf(cadena_puntos, "Puntos: %7d", score[j].puntos);
            escribir_texto(renderer, font, cadena_puntos, 575, 20, VERDE);
            #endif
            float py=60;
            if(vidas>0){
                for(size_t v=0;v<vidas;v++){
                    poligono_t *vida=poligono_crear_circulo(BOLA_RADIO,750,py);
                    if(vida==NULL){
                        break;
                    }
                    poligono_dibujar_SDL(vida,VERDE,renderer);
                    poligono_destruir(vida);
                    py+=30;
                }
            }

            if (cayendo){
                // Si la bola estÃ¡ cayendo actualizamos su posiciÃ³n
                vy = computar_velocidad(vy, G, DT);
                vx *= ROZAMIENTO;
                vy *= ROZAMIENTO;
                cx = computar_posicion(cx, vx, DT);
                cy = computar_posicion(cy, vy, DT);
            }else{
                // Si la bola no se disparÃ³ establecemos condiciones iniciales
                cx = CANON_X + CANON_LARGO * sin(canon_angulo);
                cy = CANON_Y + CANON_LARGO * cos(canon_angulo);
                vx = BOLA_VI * sin(canon_angulo);
                vy = BOLA_VI * cos(canon_angulo);
            }
            
            //Movimiento obsatculos
            lista_iter_t *iter_obs_mov = lista_iter_crear(niveles[j]);
            if(iter_obs_mov==NULL){
                break;
            }
            obstaculo_t *o_mov;

            for (size_t i = 0; i < cant_obstaculos_por_nivel[j]; i++){
                o_mov = lista_iter_ver_actual(iter_obs_mov);
                obstaculo_ejecutar_movimiento(o_mov,DT);
                lista_iter_avanzar(iter_obs_mov);
            }
            lista_iter_destruir(iter_obs_mov);

            //Rebote contra los obsatculos:

            lista_iter_t *iter_obs_choque = lista_iter_crear(niveles[j]);
            if (iter_obs_choque == NULL){
                break;
            }
            obstaculo_t *o_choque;

            float norm_x, norm_y;

            for (size_t i = 0; i < cant_obstaculos_por_nivel[j]; i++){
                o_choque = lista_iter_ver_actual(iter_obs_choque);
                if(obstaculo_distancia(o_choque,cx,cy,&norm_x,&norm_y)<=BOLA_RADIO){
                    reflejar(norm_x,norm_y,&cx,&cy,&vx,&vy);
                    vx*=PLASTICIDAD;
                    vy*=PLASTICIDAD;
                    if(obstaculo_color(o_choque)!=GRIS){
                        if(obstaculo_color(o_choque)==VERDE){
                            vidas++;
                            score[j].puntos+=(10*score[j].multiplicador);
                        }
                        if(obstaculo_color(o_choque)==NARANJA){
                            (score[j].naranjas)++;
                            score[j].multiplicador=actualizar_multiplicador(score[j].naranjas,score[j].multiplicador);
                            score[j].puntos+=100*(score[j].multiplicador);
                        }
                        if (obstaculo_color(o_choque) == AZUL || obstaculo_color(o_choque) == VERDE){
                            score[j].puntos+=(10*score[j].multiplicador);
                        }
                        obstaculo_cambiar_color(o_choque, AMARILLO);
                    }

                }
                lista_iter_avanzar(iter_obs_choque);
            }
            lista_iter_destruir(iter_obs_choque);

            // Rebote contra las paredes:
            if (cx < MIN_X + BOLA_RADIO || cx > MAX_X - BOLA_RADIO)
                vx = -vx;
            if (cy < MIN_Y + BOLA_RADIO)
                vy = -vy;

            // Se saliÃ³ de la pantalla o se traba la pelota:
            if ((cy > MAX_Y - BOLA_RADIO)){
                cayendo = false;
                lista_iter_t *iter_obs_borrar = lista_iter_crear(niveles[j]);
                if (iter_obs_borrar == NULL){
                    break;
                }
                obstaculo_t *o_borrar;
                int16_t aux= cant_obstaculos_por_nivel[j];
                for (size_t i = 0; i < cant_obstaculos_por_nivel[j]; i++){
                    o_borrar = lista_iter_ver_actual(iter_obs_borrar);
                    if(obstaculo_color(o_borrar)==AMARILLO){
                        lista_iter_borrar(iter_obs_borrar);
                        obstaculo_destruir(o_borrar);
                        aux--;
                    }else{
                    lista_iter_avanzar(iter_obs_borrar);
                    }
                }
                cant_obstaculos_por_nivel[j]=aux;
                lista_iter_destruir(iter_obs_borrar);
                if (vidas != 0){
                    vidas--;
                }
            }else if(vx > -V_MIN && vx < V_MIN){
                lista_iter_t *iter_obs_borrar1 = lista_iter_crear(niveles[j]);
                if (iter_obs_borrar1 == NULL){
                    break;
                }
                obstaculo_t *o_borrar1;
                int16_t aux = cant_obstaculos_por_nivel[j];
                for (size_t i = 0; i < cant_obstaculos_por_nivel[j]; i++){
                    o_borrar1 = lista_iter_ver_actual(iter_obs_borrar1);
                    if (obstaculo_color(o_borrar1) == AMARILLO){
                        lista_iter_borrar(iter_obs_borrar1);
                        obstaculo_destruir(o_borrar1);
                        aux--;
                    }else{
                        lista_iter_avanzar(iter_obs_borrar1);
                        }
                    }
                    cant_obstaculos_por_nivel[j] = aux;
                    lista_iter_destruir(iter_obs_borrar1);
                }

            // Dibujamos el caÃ±Ã³n:
            SDL_RenderDrawLine(renderer, CANON_X, CANON_Y, CANON_X + sin(canon_angulo) * CANON_LARGO, CANON_Y + cos(canon_angulo) * CANON_LARGO);

            // Dibujamos la bola:
            
            poligono_t *bola=poligono_crear_circulo(BOLA_RADIO,cx,cy);
            if (bola == NULL){
                break;
            }
            poligono_dibujar_SDL(bola,VERDE,renderer);
            poligono_destruir(bola);

            // Diubujamos los obstaculos
            lista_iter_t *iter_obs_dibujo = lista_iter_crear(niveles[j]);
            if (iter_obs_dibujo == NULL){
                break;
            }
            obstaculo_t *o_dibujar;
            for(size_t i=0;i<cant_obstaculos_por_nivel[j];i++){
                o_dibujar=lista_iter_ver_actual(iter_obs_dibujo);
                obstaculo_dibujar_SDL(o_dibujar,renderer);
                lista_iter_avanzar(iter_obs_dibujo);
            }
            lista_iter_destruir(iter_obs_dibujo);
            
            // Dibujamos las paredes:
            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);
            SDL_RenderDrawLine(renderer, MIN_X, MIN_Y, MAX_X, MIN_Y);
            SDL_RenderDrawLine(renderer, MIN_X, MAX_Y, MAX_X, MAX_Y);
            SDL_RenderDrawLine(renderer, MIN_X, MAX_Y, MIN_X, MIN_Y);
            SDL_RenderDrawLine(renderer, MAX_X, MAX_Y, MAX_X, MIN_Y);

            // Dibujamos el vector de velocidad:
            if(cayendo){
                SDL_RenderDrawLine(renderer, cx, cy, cx + vx, cy + vy);
            }else{
                float stepx = vx;
                float stepy = vy;
                float ux=cx;
                float uy=cy;
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                for(size_t i=0;i<130;i++){
                    SDL_RenderDrawLine(renderer, ux, uy,ux + stepx * DT,uy + stepy * DT);
                    stepy = computar_velocidad(stepy, G, DT);
                    stepx *= ROZAMIENTO;
                    stepy *= ROZAMIENTO;
                    ux = computar_posicion(ux, stepx, DT);
                    uy = computar_posicion(uy, stepy, DT);

                    if (ux < MIN_X + BOLA_RADIO || ux > MAX_X - BOLA_RADIO)
                        stepx = -stepx;
                    if (uy < MIN_Y + BOLA_RADIO)
                        stepy = -stepy;

                //Trayectoria con choques contra obstaculos, descomentar para implementar
                    //  lista_iter_t *iter_obs_choque_linea = lista_iter_crear(lista_obstaculos);
                    //  obstaculo_t *o_choque_linea;

                    //  float norm_x_linea, norm_y_linea;

                    //  for (size_t i = 0; i < cant_obstaculos; i++){
                    //      o_choque_linea = lista_iter_ver_actual(iter_obs_choque_linea);
                    //      if (obstaculo_distancia(o_choque_linea, ux, uy, &norm_x_linea, &norm_y_linea) <= BOLA_RADIO){
                    //          reflejar(norm_x_linea, norm_y_linea, &ux, &uy, &stepx, &stepy);
                    //          stepx *= PLASTICIDAD;
                    //          stepy *= PLASTICIDAD;
                    //      }
                    //      lista_iter_avanzar(iter_obs_choque_linea);
                    // }
                    // lista_iter_destruir(iter_obs_choque_linea);
                }

                //Finalizacion del Juego

                bool terminado = true;
                lista_iter_t *iter_obs_terminado = lista_iter_crear(niveles[j]);
                if (iter_obs_terminado == NULL){
                    break;
                }
                obstaculo_t *o_terminado;
                for (size_t i = 0; i < cant_obstaculos_por_nivel[j]; i++){
                    o_terminado = lista_iter_ver_actual(iter_obs_terminado);
                    if (obstaculo_color(o_terminado) == NARANJA){
                        terminado = false;
                        break;
                    }
                    lista_iter_avanzar(iter_obs_terminado);
                }
                lista_iter_destruir(iter_obs_terminado);

                if (terminado){
                    break;
                }
            }
            // END cÃ³digo del alumno

            SDL_RenderPresent(renderer);
            ticks = SDL_GetTicks() - ticks;
            if (dormir){
                SDL_Delay(dormir);
                dormir = 0;
            }
            else if (ticks < 1000 / JUEGO_FPS)
                SDL_Delay(1000 / JUEGO_FPS - ticks);
            ticks = SDL_GetTicks();
        }

        lista_destruir(niveles[j],wrapper_obstaculo_destruir);
        cayendo= false;
    }
    // BEGIN cÃ³digo del alumno
    unsigned int total=0;
    for(size_t i=1;i<=cant_niveles;i++){
        total += score[i - 1].puntos;
        printf("Nivel %zd: %d pts\n", i, score[i - 1].puntos);
    }
    printf("\nTotal = %d pts\n",total);

    free(niveles);
    free(cant_obstaculos_por_nivel);
    // END cÃ³digo del alumno

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    #ifdef TTF
    TTF_CloseFont(font);
    TTF_Quit();
    #endif
    SDL_Quit();
    return 0;
}

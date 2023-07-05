CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -c
LFLAGS = -lm -lSDL2 -lSDL2_ttf -DTTF

all: peggle

peggle: main.o lectura.o obstaculo.o lista.o poligono.o
	$(CC) -Wall main.o poligono.o lista.o lectura.o obstaculo.o -o peggle $(LFLAGS)

main.o: main.c lectura.h poligono.h obstaculo.h lista.h config.h
	$(CC) $(CFLAGS) main.c $(SDLFLAGS)

lectura.o: lectura.c lectura.h 
	$(CC) $(CFLAGS) lectura.c

obstaculo.o: obstaculo.c obstaculo.h 
	$(CC) $(CFLAGS) obstaculo.c

poligono.o: poligono.c poligono.h 
	$(CC) $(CFLAGS) poligono.c

lista.o: lista.c lista.h 
	$(CC) $(CFLAGS) lista.c

clean:
	rm *.o
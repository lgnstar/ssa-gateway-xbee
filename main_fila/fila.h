#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct{
	unsigned char *dado;
	int tam;
	int PostTent;
	int flag_atu;
}TipoFilaDado;

struct TipoFila{
	TipoFilaDado dado;
	struct TipoFila *prox;
};

typedef struct{
	struct TipoFila *p;
	struct TipoFila *u;
	int tam;
}TTFila;

void fila_insert(TTFila *fila,TipoFilaDado dado);
int fila_remove(TipoFilaDado *dado,TTFila *fila);
void fila_Packet(TTFila *fila,unsigned char *dado, int tam);
void fila_Post(TTFila *fila,unsigned char *dado, int tam,int tent,int flag);
void fila_Atu(TTFila *fila,unsigned char *dado);

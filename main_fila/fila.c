#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fila.h"


void fila_FVazia(TTFila *fila){
	struct TipoFila *aux=(struct TipoFila*)malloc(sizeof(struct TipoFila));
	fila->p=aux;
	fila->u=fila->p;
	fila->p->prox=NULL;
	fila->tam=0;
}
void fila_insert(TTFila *fila,TipoFilaDado dado){
	struct TipoFila *aux;//=(struct TipoFila*)malloc(sizeof(struct TipoFila));

	if(fila->tam==0){
		fila->p->dado=dado;
		//fila->tam=1;
		}
	else{
		aux=(struct TipoFila*)malloc(sizeof(struct TipoFila));
		aux->dado=dado;
		fila->u->prox=aux;
		fila->u=fila->u->prox;
		aux->prox=NULL;

		}
	fila->tam++;

}
void fila_imprime(TTFila *fila){
	int i,j=0;
	struct TipoFila *aux=fila->p->prox;
	while(aux != NULL){

		printf("item %d tam= %d\n",j,aux->dado.tam);
		printf("packet: ");
		for(i=0;i<aux->dado.tam;i++)
			printf("%02X ",aux->dado.dado[i]);
		printf("\n\n");
		aux=aux->prox;
		j++;
	}
}
int fila_remove(TipoFilaDado *dado,TTFila *fila){
	//if(fila->p->prox == NULL){

		struct TipoFila *aux;//=fila->p;
		//fila->p=fila->p->prox;

		if(fila->tam>1){
			aux=fila->p;
			fila->p=fila->p->prox;
			*dado=aux->dado;
			free(aux);
			}
		else{
			fila->p=fila->u;
			*dado=fila->p->dado;
			}
		if(fila->tam > 0) fila->tam--;
		return 1;//fila->tam;
		//}
	//return 0;
}
int fila_count(TTFila *fila){
	int count=0;
	struct TipoFila *aux=fila->p->prox;
		while(aux != NULL){
			count++;
			aux=aux->prox;
		}
	return count;
}
void fila_Packet(TTFila *fila,unsigned char *dado, int tam){
	TipoFilaDado dat;
	dat.dado=(unsigned char*)malloc(sizeof(unsigned char)*(tam+1));
	//memcpy(dat.dado,dado,tam);
	dat.dado=dado;
	dat.tam=tam;
	fila_insert(fila,dat);
}
void fila_Post(TTFila *fila,unsigned char *dado, int tam,int tent,int flag){
	TipoFilaDado dat;
	dat.dado=(unsigned char*)malloc(sizeof(unsigned char)*(tam+1));
	//memcpy(dat.dado,dado,tam);
	dat.dado=dado;
	dat.tam=tam;
	dat.PostTent=tent;
	dat.flag_atu=flag;
	fila_insert(fila,dat);
}
void fila_Atu(TTFila *fila,unsigned char *dado){
	TipoFilaDado dat;
	dat.dado=(unsigned char*)malloc(sizeof(unsigned char)*((strlen(dado))));
	//memcpy(dat.dado,dado,tam);
	dat.dado=dado;
	fila_insert(fila,dat);
}

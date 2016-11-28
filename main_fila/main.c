#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "fila.h"
#include "serial.h"
#include "xbee.h"
#include "cod.h"
#include "ssa.h"
#include "socket.h"
#include "ssl.h"

#define DEBUG_INIT
#define DEBUG_SER_BUF		0
#define DEBUG_XBEE_PACK
#define DEBUG_POST


#define START 	1
#define END		2

///#define	socket_post1(a,b) socket_SSL(a,b,xbee.myaddr)
void xbee_atu(uint8_t *buf1);

TTFila FilaPacket;
TTFila FilaPost;
TTFila FilaAtu;
pthread_mutex_t lock_Atu = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_Post = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_Packet = PTHREAD_MUTEX_INITIALIZER;
typedef struct{

	}thread_args;
//variaveis globais

//serial
int fd;
//flags
int flag_ser=0;
int flag_process=0;
int flag_atu=0;
uint8_t BufAtu[100];
//mestra
xbee_t xbee;
long timeout(struct timeval start){
	long mtime=0, seconds=0, useconds=0;
	struct timeval end;
	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	return mtime;
}
int checksum(uint8_t *buf,int tam){
	int i=0,sum=0;
	sum=0;
	for(i=3;i<tam-1;i++){
		sum+=buf[i];
		}
	sum&=0xFF;
	sum=0xFF-sum;
	if(sum==buf[tam-1]) return 1;
	return 0;
}
int init(uint8_t *buf,int tam){
	int i=0;
	for(i=0;i<tam;i++){
		if(buf[i]==0x7E) return i;
	}
	return -1;
}

void signal_handler_IO(int status)
{
flag_ser=1;
}
void *ThreadSerial(void *vargp){
	int status=0,ind=0,ind1=0,sum=0,tam=0,i=0,res=0,flag_ini=0,j=0;
	uint8_t bufin[200],bufaux[100],c[2];
	fila_FVazia(&FilaPacket);
	fd=serial_init(&signal_handler_IO);

	xbee.flag_myaddr=1;
	xbee_cmdAT(fd,&xbee,(uint8_t*)"SH");

	while(1){
		if(flag_ser){
#ifdef DEBUG_INIT
			printf("ser\n");
			fflush(stdout);
#endif
			res = read(fd,bufin,200);

#ifdef DEBUG_SER_BUF
			printf("buf= ");
			fflush(stdout);
			for(j=0;j<res;j++){
				printf("%02X ",bufin[j]);
				fflush(stdout);
				}
			printf("\n");
			fflush(stdout);
#endif

			for(i=0;i<res;i++){
				switch(status){
					case 0:
						//printf("status=0\n");
						//fflush(stdout);
						if(bufin[i]==0x7e){
							ind=0;
							memset(bufaux,0,sizeof(bufaux));
							bufaux[0]=0x7e;
							status=1;
							}
						break;
					case 1:
						ind++;
						bufaux[ind]=bufin[i];
						if(ind==2) tam=(bufaux[2] | bufaux[1]<<8)+4;

						else if(ind>2 & ind==(tam-1)){
							//printf("teste1 ind=%d\n");
							//fflush(stdout);
							if(checksum(bufaux,tam)){
								printf("checksum fila=%d\n",FilaPacket.tam);
								fflush(stdout);
								pthread_mutex_lock(&lock_Packet);
								fila_Packet(&FilaPacket,bufaux,tam);
								pthread_mutex_unlock(&lock_Packet);
								status=0;
								}
							else status=0;
							}
						break;
					}
				}
			flag_ser=0;
			}
		usleep(35);
		}
	}
void discover(void){
	xbee_cmdAT(fd,&xbee,(uint8_t*)"ND");
	xbee.disc.qtd=0;
	gettimeofday(&(xbee.disc.start), NULL);
	//discover.discover_tim_start=TIM2->CNT;
	xbee.disc.flag=1;
}
void *ThreadXbee(void *vargp){
	int i,j;
	TipoFilaDado dado;
	floatbyte_T floataux;
	uint8_t buf[200],buf2[10][200],buf1[100];
	uint8_t addraux[8];
	uint8_t payload[50];
	struct timeval start;
	fila_FVazia(&FilaPost);
	fila_FVazia(&FilaAtu);
	gettimeofday(&start, NULL);
	while(1){
		if(FilaAtu.tam>0){
#ifdef DEBUG_INIT
			printf("xbee atu\n");
			fflush(stdout);
#endif
			pthread_mutex_lock(&lock_Atu);
			fila_remove(&dado,&FilaAtu);
			pthread_mutex_unlock(&lock_Atu);

			xbee_atu(dado.dado);
		}
		//discover
		if((timeout(xbee.disc.start)>150000) & (xbee.disc.flag)){
#ifdef DEBUG_INIT
			printf("xbee discovery\n");
			fflush(stdout);
#endif
			//monta pacote para enviar todos os dispositivos presentes
			sprintf((char*)buf,"act=discover&mac=%s&qtd=%d",(char*)xbee.myaddr,xbee.disc.qtd);
			for(i=0;i<xbee.disc.qtd;i++){
				sprintf((char*)buf2[1],"&mac%d=%02X%02X%02X%02X%02X%02X%02X%02X"
										,i,
										xbee.disc.addr[i][0],
										xbee.disc.addr[i][1],
										xbee.disc.addr[i][2],
										xbee.disc.addr[i][3],
										xbee.disc.addr[i][4],
										xbee.disc.addr[i][5],
										xbee.disc.addr[i][6],
										xbee.disc.addr[i][7]);

				strcat((char*)buf,(char*)buf2[1]);
				}
			pthread_mutex_lock(&lock_Post);
			fila_Post(&FilaPost,buf,strlen(buf),2,0);
			pthread_mutex_unlock(&lock_Post);

			gettimeofday(&xbee.disc.start, NULL);
		}
		//disp_atu

		//verifica se existe pacote na fila da serial
		if(FilaPacket.tam>0){
#ifdef DEBUG_INIT
			printf("xbee\n");
			fflush(stdout);
#endif
			//remove da fila e copia para a estrutura do xbee
			pthread_mutex_lock(&lock_Packet);
			fila_remove(&dado,&FilaPacket);
			pthread_mutex_unlock(&lock_Packet);

			memcpy(xbee.buf,dado.dado,dado.tam);
			xbee_reciver(&xbee);

#ifdef DEBUG_XBEE_PACK
			printf("packet rec: ");
			fflush(stdout);
			for(i=0;i<xbee.buf[2]+4;i++){
				printf("%02x ",xbee.buf[i]);
				fflush(stdout);
				}
			printf("\n");
			fflush(stdout);
#endif
			switch(xbee.buf[3]){
				case XBEE_STATUS_SUCESS:
					break;
				case XBEE_CMDATRR:
					//recebe comando remoto

					//recebe o rssi de cada modulo
					if((xbee.buf[15]=='D') & (xbee.buf[16]=='B')){
						memset(buf,0,sizeof(buf));
						sprintf((char*)buf,"act=rssi&rssi=%d",xbee.buf[18]);

						sprintf((char*)buf,"%s&mac=%02X%02X%02X%02X%02X%02X%02X%02X",
								(char*)buf,
								xbee.buf[5+0],
								xbee.buf[5+1],
								xbee.buf[5+2],
								xbee.buf[5+3],
								xbee.buf[5+4],
								xbee.buf[5+5],
								xbee.buf[5+6],
								xbee.buf[5+7]);

						//printf("\nbuf=%s\n\n",buf);
						pthread_mutex_lock(&lock_Post);
						fila_Post(&FilaPost,buf,strlen(buf),2,1);
						pthread_mutex_unlock(&lock_Post);
						}
					break;
				case XBEE_CMDAT:
					//recebe comando local

					//primeiro estagio para obten��o do MAC
					if(xbee.flag_myaddr==1){
						//zera o MAC
						memset(xbee.myaddres,0,sizeof(xbee.myaddres));

						for(i=0;i<4;i++)
							xbee.myaddres[i]=xbee.buf[8+i];
						xbee.flag_myaddr=2;
						xbee_cmdAT(fd,&xbee,(uint8_t*)"SL");
						}
					//segundo estagio para obten��o do MAC
					else if(xbee.flag_myaddr==2){

						for(i=0;i<4;i++)
							xbee.myaddres[4+i]=xbee.buf[8+i];
						xbee.flag_myaddr=3;
						//converte vetor do MAC para string
						xbee_addrstr(xbee.myaddres,xbee.myaddr);

						sprintf((char*)buf,"act=init&type=5&mac=%s",(char*)xbee.myaddr);
						//printf("myaddr\n");
						pthread_mutex_lock(&lock_Post);
						fila_Post(&FilaPost,buf,strlen(buf),5,0);
						pthread_mutex_unlock(&lock_Post);

						discover();
						}
					//recebe o MAC dos dispositivo pelo discover
					if((xbee.buf[5]=='N') & (xbee.buf[6]=='D')){
						for(i=0;i<8;i++){
							xbee.disc.addr[xbee.disc.qtd][i]=xbee.buf[10+i];
							addraux[i]=xbee.buf[10+i];
							}
						//usleep(100);
						//printf("discover\n");
						//pede o rssi do dispositivo
						xbee_cmdATR(fd,&xbee,(uint8_t*)"DB",addraux);
						//incrementa a quantidade de dispositivos
						xbee.disc.qtd++;
					}
					break;
				//recebe um pacote de dados
				case XBEE_RECEIVE_PACKET:
					switch(xbee.buf[XBEE_PAYLOAD_OFFSET]){
						//recebe a altera��o de sinal de saida
						case SSA_F_OUTP:
							//pega o endere�o de onde veio
							xbee_addrstr(xbee.source_Address,buf1);

							//monta o pacote para envio para o servidor
							sprintf((char*)buf,"act=est&out=%d&est=%d&mac=%s",
									xbee.buf[XBEE_PAYLOAD_OFFSET+1],
									xbee.buf[XBEE_PAYLOAD_OFFSET+2],
									(char*)buf1);

							pthread_mutex_lock(&lock_Post);
							fila_Post(&FilaPost,buf,strlen(buf),2,0);
							pthread_mutex_unlock(&lock_Post);

							break;

						//envia o endere�o do coordenador para o dispositivo
						case SSA_F_CORD_ADDR:
							//printf("intit disp\n");
							payload[0]=SSA_F_CORD_ADDRP;
							for(i=0;i<8;i++)
								payload[1+i]=xbee.myaddres[i];
							xbee_SendData(fd,&xbee,xbee.source_Address,payload,9);

							break;
						//recebe dado de entrada analogica
						case SSA_F_ANALOG:

							memset(buf,0,sizeof(buf));

							//grava o MAC de onde veio o dado
							xbee_addrstr(xbee.source_Address,(uint8_t*)buf2[0]);

							//monta o pacote para envio do pacote
							sprintf((char*)buf,"act=var1&disptype=3&mac=%s&qtd=%d",
									buf2[0],
									xbee.buf[XBEE_PAYLOAD_OFFSET+1]);

							for(i=0;i<xbee.buf[XBEE_PAYLOAD_OFFSET+1];i++){
								//coloca os 4 byte no union para passar para float
								for(j=0;j<4;j++)
									floataux.byte.b[j]=xbee.buf[XBEE_PAYLOAD_OFFSET+6+j+i*8];
								//insere os dados de cada variavel no pacote
								sprintf(buf2[0],"&id%d=%d&tempo%d=%d&type%d=%d&val%d=%2.3f&media%d=%d",
										i,
										xbee.buf[XBEE_PAYLOAD_OFFSET+2+i*8],
										i,
										xbee.buf[XBEE_PAYLOAD_OFFSET+3+i*8],
										i,
										xbee.buf[XBEE_PAYLOAD_OFFSET+4+i*8],
										i,
										floataux.val,
										i,
										xbee.buf[XBEE_PAYLOAD_OFFSET+5+i*8]);
								strcat((char*)buf,(char*)buf2[0]);

								}
							pthread_mutex_lock(&lock_Post);
							fila_Post(&FilaPost,buf,strlen(buf),2,1);
							pthread_mutex_unlock(&lock_Post);

							break;
						}
					break;
				}
#ifdef DEBUG_INIT
			printf("xbeeend\n");
			fflush(stdout);
#endif
			}
		usleep(1000);
		}
	}
void *ThreadPost(void *vargp){
	TipoFilaDado dado,dado1;
	int tent=0;
	uint8_t buf[100],buf1[100];
	int total=0,certo=0;
	while(1){
		if(FilaPost.tam>0){
#ifdef DEBUG_INIT
			printf("post\n");
			fflush(stdout);
#endif

			tent=0;

			pthread_mutex_lock(&lock_Post);
			fila_remove(&dado,&FilaPost);
			pthread_mutex_unlock(&lock_Post);

			//printf("Posttam=%d\n",FilaPost.tam);
			//fflush(stdout);
#ifdef DEBUG_POST
			printf("post:\"%s\"\n",dado.dado);
			fflush(stdout);
#endif
			total++;
			while(!socket_post1((char*)dado.dado,(char*)&buf1) && tent<dado.PostTent){
				tent++;
				//printf("error post %d\n",tent);
				//fflush(stdout);
				usleep(500000);
				}
			if(tent<dado.PostTent){
				certo++;
				printf("buf1=%s\n",buf1);
				fflush(stdout);
				if(dado.flag_atu){
					pthread_mutex_lock(&lock_Atu);
					fila_Atu(&FilaAtu,buf1);
					pthread_mutex_unlock(&lock_Atu);
					}
				//fflush(stdout);
				//printf("buff\n");
				}
			if(total==50){
				sprintf(buf,"act=perca&mac=%s&perca=%2.2f",xbee.myaddr,(1.0-((float)certo/(float)total))*100.0);
				pthread_mutex_lock(&lock_Post);
				fila_Post(&FilaPost,buf,strlen(buf),4,0);
				pthread_mutex_unlock(&lock_Post);
				total=0;
				certo=0;
				}
#ifdef DEBUG_INIT
			printf("postend\n");
			fflush(stdout);
#endif
			}
		usleep(100000);
		}
	}
int main(){
	int i,id;
	thread_args args;
	pthread_t IdSerial,IdXbee,IdPost;

	TipoFilaDado dado;
	struct timeval start,start1;

	printf("inicio\n");
	fflush(stdout);
	pthread_create(&(IdSerial), NULL, ThreadSerial, (void *)&(args));
	pthread_create(&(IdXbee), NULL, ThreadXbee, (void *)&(args));
	pthread_create(&(IdPost), NULL, ThreadPost, (void *)&(args));

	gettimeofday(&start, NULL);
	gettimeofday(&start1, NULL);
	while(1){
		usleep(10000000);
	}
}
void xbee_atu(uint8_t *buf1){
	int i,j,k;
	uint8_t buf[100],payload[100],addraux[50];
	memset(buf,0,sizeof(buf));

	i=0;
	//espera o inicio
	while((buf1[i]<'0') | (buf1[i]>'9')) i++;
	j=i;
	i=0;
	//pega o numero de variaveis
	while(buf1[i+j]!=';'){
		buf[i]=buf1[i+j];
		i++;
		}
	int qtd=atoi((char*)buf);
	j+=i+1;
	//se tiver alguma variavel para atualizar
	if(qtd>0){
		memset(buf,0,sizeof(buf));
		i=0;
		//vai ate o proximo ; que tem o MAC do dispositivo
		while(buf1[i+j]!=';'){
			buf[i]=buf1[i+j];
			i++;
			}

		memset(addraux,0,sizeof(addraux));
		//extrai o MAC
		sscanf((char*)buf,"%02X%02X%02X%02X%02X%02X%02X%02X",
				(unsigned int*)&addraux[0],(unsigned int*)&addraux[1],
				(unsigned int*)&addraux[2],(unsigned int*)&addraux[3],
				(unsigned int*)&addraux[4],(unsigned int*)&addraux[5],
				(unsigned int*)&addraux[6],(unsigned int*)&addraux[7]);

		i=0;
		while(buf1[i+j]!=';') i++;
		j+=i+1;

		//inicia o pacote para enviar para o dispositivo
		memset(payload,0,sizeof(payload));
		payload[0]=SSA_F_ANALOG_TEMPO;
		payload[1]=qtd;

		for(k=0;k<qtd;k++){

			//variavel
			memset(buf,0,sizeof(buf));
			i=0;
			while(buf1[i+j]!=';'){
				buf[i]=buf1[i+j];
				i++;
				}
			//insere o id da variavel na pacote
			payload[2+k*3]=atoi((char*)buf);

			//tempo
			memset(buf,0,sizeof(buf));
			j+=i+1;
			i=0;
			while(buf1[i+j]!=';'){
				buf[i]=buf1[i+j];
				i++;
				}
			//insere o tempo no pacote
			payload[3+k*3]=atoi((char*)buf);

			//medias
			memset(buf,0,sizeof(buf));
			j+=i+1;
			i=0;
			while(buf1[i+j]!=';'){
				buf[i]=buf1[i+j];
				i++;
				}
			//insere o valor das medias no pacote
			payload[4+k*3]=atoi((char*)buf);
			j+=i+1;
			}
		//envia o pacote para o dispositivo
		xbee_SendData(fd,&xbee,addraux,payload,2+3*qtd);
		}
}

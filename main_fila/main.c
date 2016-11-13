#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "fila.h"
#include "serial.h"
#include "xbee.h"
#include "cod.h"
#include "ssa.h"
//#include "socket.h"
#include "ssl.h"

#define START 	1
#define END		2

#define	socket_post1(a,b) socket_SSL(a,b,xbee.myaddr)
void xbee_atu(uint8_t *buf1);

TTFila FilaPacket;
TTFila FilaPost;
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
void signal_handler_IO (int status)
{
flag_ser=1;
}
int checksum(uint8_t *buf,int tam){
	int i,sum=0;
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
	int i;
	for(i=0;i<tam;i++){
		if(buf[i]==0x7E) return i;
	}
	return -1;
}
void *ThreadSerial(void *vargp){
	int status=0,ind=0,ind1=0,sum=0,tam=0,i=0,res,flag_ini=0,j;
	uint8_t bufin[100],bufaux[100];
	fila_FVazia(&FilaPacket);
	fd=serial_init(&signal_handler_IO);

	xbee.flag_myaddr=1;
	xbee_cmdAT(fd,&xbee,(uint8_t*)"SH");

	while(1){
		if(flag_ser){
			memset(bufin,0,sizeof(bufin));
			res = read(fd,bufin,100);
			/*printf("buf= ");
			fflush(stdout);
			for(j=0;j<res;j++){
				printf("%02X ",bufin[j]);
				fflush(stdout);
				}
			printf("\n");
			fflush(stdout);*/
			if(flag_ini){
				//printf("flag_ini\n");
				if(ind<3){
					//printf("flag_ini1\n");
					i=0;
					//while(i<res){
					while(bufin[i]!=0x7E & i<res){
						bufaux[ind]=bufin[i];
						ind++;
						i++;
					}
					tam=(bufaux[2] | bufaux[1]<<8)+4;

					if(checksum(bufaux,tam)){
						printf("ok1\n");
						//fflush(stdout);
						fila_Packet(&FilaPacket,bufaux,tam);
						ind=2;
						//i++;
						if(i==res) flag_ini=0;
						bufaux[0]=0x7e;
						//printf("tam=%d res=%d ind=%d ind=%02X ind1=%02X ind2=%02X ini\n",tam,res,ind,bufin[i],bufin[i+1],bufin[i+2]);
					}
				}
				if((res+ind)>tam){
					//printf("flag_ini2\n");
					//i=0;
					//printf("tam=%d res=%d ind=%d ind=%02X ind1=%02X ind2=%02X %dini\n",tam,res,ind,bufin[i],bufin[i+1],bufin[i+2],i);
					//ind--;
					while(i<res){
						while(bufin[i]!=0x7E & i<res){
							bufaux[ind-1]=bufin[i];
							ind++;
							i++;
							}
						if(checksum(bufaux,tam)){
							fila_Packet(&FilaPacket,bufaux,tam);
							printf("ok2\n");
							//fflush(stdout);
							//printf("\n ok flag_ini i=%d res=%d\n",i,res);
							//flag_ini=0;
							if((i+3)<res){
								ind=2;
								bufin[i]=0;
								bufaux[0]=0x7e;
								tam=(bufin[i+2] | bufin[i+1]<<8)+4;

								//printf("tam=%d res=%d ind=%d ind=%02X ind1=%02X ind2=%02X ini\n",tam,res,ind,bufin[i],bufin[i+1],bufin[i+2]);
								i++;
								}
							else
								flag_ini=0;
							}
						}


					}
				}
			if(res>=1 & bufin[0]==0x7E){
				//printf("res1\n");
				ind=0;
				tam=0;
				if(res>2){
					tam=(bufin[2] | bufin[1]<<8)+4;
					}
				else{
					//printf("teste1\n");
					for(i=0;i<res;i++)
						bufaux[i]=bufin[i];
					ind=res;
					flag_ini=1;
				}
			//	printf("res=%d tam=%d\n",res,tam);
				if(res==tam & checksum(bufin,tam)){
					//printf("tam=res ok\n");
					memset(xbee.buf,0,sizeof(xbee.buf));
					for(i=0;i<=tam;i++)
						xbee.buf[i]=bufin[i];
					printf("ok3\n");
					//fflush(stdout);
					//printf("ok tam=%d\n",tam);
					fila_Packet(&FilaPacket,bufin,tam);
					}
				if(res<tam){
					//printf("res2\n");
					flag_ini=1;
					ind=0;
					i=0;
					while(bufin[ind+1]!=0x7E & ind<=res){
						bufaux[ind]=bufin[ind];
						ind++;
						}
					}
				else
				if(res>tam & res>2){
					//printf("res3\n");
					ind=1;
					i=1;
					bufaux[0]=0x7E;
					while(ind<res){
						while(bufin[ind]!=0x7E & ind<res){
							bufaux[i]=bufin[ind];
							ind++;
							i++;
							}
						if(checksum(bufaux,tam)){
							printf("\nok4\n");
							//fflush(stdout);
							ind++;
							i=1;
							bufaux[0]=0x7e;
							}
						}
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
	gettimeofday(&start, NULL);
	while(1){
		if(flag_atu){
			xbee_atu(BufAtu);
			flag_atu=0;
		}
		//discover
		if((timeout(xbee.disc.start)>150000) & (xbee.disc.flag)){
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
			fila_Post(&FilaPost,buf,strlen(buf),2,0);
			gettimeofday(&xbee.disc.start, NULL);
		}
		//disp_atu

		//verifica se existe pacote na fila da serial
		if(FilaPacket.tam>0){
			//remove da fila e copia para a estrutura do xbee
			fila_remove(&dado,&FilaPacket);
			memcpy(xbee.buf,dado.dado,dado.tam);
			xbee_reciver(&xbee);
			printf("packet rec: ");
			fflush(stdout);
			for(i=0;i<xbee.buf[2]+4;i++){
				printf("%02x ",xbee.buf[i]);
				fflush(stdout);
				}
			printf("\n");
			//fflush(stdout);
			//printf("xbeetam=%d\n",FilaPacket.tam);
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
						fila_Post(&FilaPost,buf,strlen(buf),2,1);
						}
					break;
				case XBEE_CMDAT:
					//recebe comando local

					//primeiro estagio para obtenção do MAC
					if(xbee.flag_myaddr==1){
						//zera o MAC
						memset(xbee.myaddres,0,sizeof(xbee.myaddres));

						for(i=0;i<4;i++)
							xbee.myaddres[i]=xbee.buf[8+i];
						xbee.flag_myaddr=2;
						xbee_cmdAT(fd,&xbee,(uint8_t*)"SL");
						}
					//segundo estagio para obtenção do MAC
					else if(xbee.flag_myaddr==2){

						for(i=0;i<4;i++)
							xbee.myaddres[4+i]=xbee.buf[8+i];
						xbee.flag_myaddr=3;
						//converte vetor do MAC para string
						xbee_addrstr(xbee.myaddres,xbee.myaddr);

						sprintf((char*)buf,"act=init&type=5&mac=%s",(char*)xbee.myaddr);
						//printf("myaddr\n");
						fila_Post(&FilaPost,buf,strlen(buf),5,1);
						discover();
						}
					//recebe o MAC dos dispositivo pelo discover
					if((xbee.buf[5]=='N') & (xbee.buf[6]=='D')){
						for(i=0;i<8;i++){
							xbee.disc.addr[xbee.disc.qtd][i]=xbee.buf[10+i];
							addraux[i]=xbee.buf[10+i];
							}
						//usleep(100);
						printf("discover\n");
						//pede o rssi do dispositivo
						xbee_cmdATR(fd,&xbee,(uint8_t*)"DB",addraux);
						//incrementa a quantidade de dispositivos
						xbee.disc.qtd++;
					}
					break;
				//recebe um pacote de dados
				case XBEE_RECEIVE_PACKET:
					switch(xbee.buf[XBEE_PAYLOAD_OFFSET]){
						//recebe a alteração de sinal de saida
						case SSA_F_OUTP:
							//pega o endereço de onde veio
							xbee_addrstr(xbee.source_Address,buf1);

							//monta o pacote para envio para o servidor
							sprintf((char*)buf,"act=est&out=%d&est=%d&mac=%s",
									xbee.buf[XBEE_PAYLOAD_OFFSET+1],
									xbee.buf[XBEE_PAYLOAD_OFFSET+2],
									(char*)buf1);

							fila_Post(&FilaPost,buf,strlen(buf),2,0);

						break;

						//envia o endereço do coordenador para o dispositivo
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
							fila_Post(&FilaPost,buf,strlen(buf),2,1);

							break;
						}
					break;
				}
			}
		usleep(1000);
		}
	}
void *ThreadPost(void *vargp){
	TipoFilaDado dado;
	int tent=0;
	uint8_t buf[100],buf1[100];
	int total=0,certo=0;
	while(1){
		if(FilaPost.tam>0){
			tent=0;
			fila_remove(&dado,&FilaPost);
			printf("Posttam=%d\n",FilaPost.tam);
			//fflush(stdout);
			//printf("post:\"%s\"\n",dado.dado);
			total++;
			while(!socket_post1((char*)dado.dado,(char*)&buf1) && tent<dado.PostTent){
				tent++;
				printf("error post %d\n",tent);
				//fflush(stdout);
				usleep(500000);
				}
			if(tent<dado.PostTent){
				certo++;
				printf("buf1=%s\r\n",buf1);
				//fflush(stdout);
				if(dado.flag_atu){
					memcpy(BufAtu,buf1,strlen(buf1));
					flag_atu=1;
					dado.flag_atu=0;
					}
				fflush(stdout);
				}
			if(total==50){
				sprintf(buf,"act=perca&mac=%s&perca=%2.2f",xbee.myaddr,(1.0-((float)certo/(float)total))*100.0);
				fila_Post(&FilaPost,buf,strlen(buf),4,0);
				total=0;
				certo=0;
				}
			}
		usleep(10000);
		}
	}
int main(){
	int i;
	thread_args args;
	pthread_t IdSerial,IdXbee,IdPost;

	TipoFilaDado dado;
	struct timeval start,start1;

	pthread_create(&(IdSerial), NULL, ThreadSerial, (void *)&(args));
	pthread_create(&(IdXbee), NULL, ThreadXbee, (void *)&(args));
	pthread_create(&(IdPost), NULL, ThreadPost, (void *)&(args));

	gettimeofday(&start, NULL);
	gettimeofday(&start1, NULL);
	while(1){
		if(timeout(start)>500){
			//printf("FilaPacket tam=%d tam1=%d\n",FilaPacket.tam,FilaPacket.p->dado.tam);
			gettimeofday(&start, NULL);
		}
		//if(timeout(start1)>10000){
		/*if(FilaPacket.tam>0){
			if(fila_remove(&dado,&FilaPacket)){

				printf("tamfila=%d tamdado=%d\n\n",FilaPacket.tam,dado.tam);

				printf("packet: ");
				for(i=0;i<dado.tam;i++){
					printf("%02X ",dado.dado[i]);
					}
				printf("\n");
				}
			gettimeofday(&start1, NULL);
			}*/
		usleep(100000);
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

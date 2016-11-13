/********************************************************************
 **************************uart_test*********************************
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>


#include "serial.h"
#include "xbee.h"
#include "cod.h"
#include "ssa.h"
//#include "socket.h"
#include "ssl.h"


#define START 	1
#define END		2

//estrutura de controle e auxilio para o XBEE
xbee_t xbee;

#define	socket_post1(a,b) socket_SSL(a,b,xbee.myaddr)


//EndereÃ§o para broadcast da rede XBEE
uint8_t Addr_broadcast[]={0,0,0,0,0,0,0xFF,0xFF};

//processa pacote vindo da serial do XBEE
int ser_receb(void);
//processa os comandos vindo do XBEE
void xbee_process(void);
//verifica o timeout
long timeout(struct timeval start);
//Envia comando de discover do XBEE (ND) e inicia a contagem do tempo para o timeout
void discover(void);
//verifica de acordo com a resposta do POST de existe alguma coisa para atualizar
void atu_process(void);
//indica dados na serial
int flag_ser=0;
void signal_handler_IO (int status)
{
flag_ser=1;
}
//buffer da serial
char buf_rx[100];
//pacote do XBEE
uint8_t payload[100];
//arquivo da serial
int fd;
//union para facilitar a transferencia de float para 4 bytes e vice versa
floatbyte_T floataux;
//variaveis de controle do processo da serial
int status1=0,res,tam,ind=0,ind1=0;
//flag que indica que existe pacote do XBEE a ser processado
int flag_process=0;
//buffers de uso geral
uint8_t buf[200];
uint8_t buf1[200];
char buf2[10][200];
uint8_t addraux[8];
//processa a entrada da serial (XBEE)
void ser_process();

typedef struct{
	float analog[20];
	uint8_t var[20];
	uint8_t mac[8];
}tipo_analog;
typedef struct{
	int tipo;
	tipo_analog analog;
}pacote_T;
struct timeval start,start1;
int main(int argc, char **argv)
{

	pacote_T pacote;
	int i;
	char myaddr[20];
	//inicia a serial
	fd=serial_init(&signal_handler_IO);
	sleep(1);

	//pergunta qual é o MAC do XBEE mestre
	xbee.flag_myaddr=1;
	xbee_cmdAT(fd,&xbee,(uint8_t*)"SH");

	printf("size=%d \n",sizeof(pacote_T));
	gettimeofday(&start, NULL);
	gettimeofday(&start1, NULL);
	while(1){
		ser_process();
		xbee_process();
		//recebe o MAC do XBEE mestre
		if(xbee.flag_myaddr==3){

			//monta o pacote de iniciação do gateway no servidor
			sprintf((char*)buf,"act=init&type=5&mac=%s",(char*)xbee.myaddr);

			int tent=1;
			gettimeofday(&start, NULL);
			while(!socket_post1((char*)buf,(char*)&buf1) && tent!=5){
				tent++;
				printf("erro para se conectar tentativa %d\n",tent);
				sleep(2);
				}
			if(tent<5){
				printf("buf1=%s\r\n",buf1);
				fflush(stdout);
				}
			else{
				printf("erro ao iniciar Gateway");
				break;
			}
			printf("tempo=%d \n\n",timeout(start));
			//fim da descoberta do MAC
			xbee.flag_myaddr=0;
			//envia sinal para descobrir dispositivos
			discover();
			}
		//espera 150 sec para todos os dispositivos responderem ao discover
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

			gettimeofday(&start, NULL);
			if(socket_post1((char*)buf,(char*)&buf1)){
				printf("buf1=%s\r\n",buf1);
				fflush(stdout);
				}
			printf("tempo=%d \n\n",timeout(start));
			//reenvia o sinal de discover
			discover();
			}

		usleep(1000);
		}

					
	}

long timeout(struct timeval start){
	long mtime=0, seconds=0, useconds=0;
	struct timeval end;
	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	return mtime;
}
//manda sinal de discover
void discover(void){
	xbee_cmdAT(fd,&xbee,(uint8_t*)"ND");
	xbee.disc.qtd=0;
	gettimeofday(&(xbee.disc.start), NULL);
	//discover.discover_tim_start=TIM2->CNT;
	xbee.disc.flag=1;
}
//processa mensagem vindo da serial
void xbee_process(void){
	int i,j;
	if(flag_process){
		xbee_reciver(&xbee);
		printf("packet rec: ");
		fflush(stdout);
		for(i=0;i<xbee.buf[2]+4;i++){
			printf("%02x ",xbee.buf[i]);
			fflush(stdout);
			}
		printf("\n");
		fflush(stdout);
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


					if(socket_post1((char*)buf,(char*)&buf1)){
						printf("buf1=%s\r\n",buf1);
						fflush(stdout);

						//caso tenha recebido alguma alteração na configuração do dispositivo
						//processa e envia para o dispositivo
						atu_process();
						}
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
					}
				//recebe o MAC dos dispositivo pelo discover
				if((xbee.buf[5]=='N') & (xbee.buf[6]=='D')){
					for(i=0;i<8;i++){
						xbee.disc.addr[xbee.disc.qtd][i]=xbee.buf[10+i];
						addraux[i]=xbee.buf[10+i];
						}

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

						if(socket_post1((char*)buf,(char*)buf1)){
							printf("buf1=%s\r\n",buf1);
							fflush(stdout);
							}

					break;

					//envia o endereço do coordenador para o dispositivo
					case SSA_F_CORD_ADDR:
						printf("intit disp\n");
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
						gettimeofday(&start, NULL);
						if(socket_post1((char*)buf,(char*)buf1)){
							printf("buf1=%s\n\n",buf1);
							//atualiza o dispositivo caso necessario
							printf("tempo=%d \n\n",timeout(start));
							atu_process();
							}

						break;
					}
				break;
			}
		flag_process=0;
		}
}


//verifica a necessidade de atualizar o dispositico com configuração nova
void atu_process(void){
	int i,j,k;
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
void ser_process(){
	int flag_ini=0,flag_cont=1;
	int ref=0;
	int j,tam,i;
	if(flag_ser){
		memset(buf,0,sizeof(buf));
		res = read(fd,buf,100);
		printf("buf= ",res,ind,status1);
		fflush(stdout);
		for(i=0;i<res;i++){
			printf("%02x ",buf[i]);
			fflush(stdout);
			}
		printf("\n");
		fflush(stdout);
		flag_cont=1;
	//while(flag_cont>=1){
		//printf("loop\n");
		if(res>3 & buf[0]==0x7E){
			ind=0;
			tam=(buf[ref+2] | buf[ref+1]<<8)+4;
			printf("res=%d tam=%d\n",res,tam);
			if(res==tam & checksum(buf,tam)){
				printf("tam=res ok\n");
				memset(xbee.buf,0,sizeof(xbee.buf));
				for(j=0;j<=tam;j++)
					xbee.buf[j]=buf[j];
				flag_process=1;
				flag_cont=0;
				}
			if(res<tam){
				while(buf[ind]!=0x7E & ind<=res){
					buf1[ind]=buf[ind];
					ind++;
					}
				}
			if(res>tam){
				while(buf[ind]!=0x7E){
					buf1[ind]=buf[ind];
					ind++;
					}
				if(checksum(buf1,ind)){
					memset(xbee.buf,0,sizeof(xbee.buf));
					for(j=0;j<=tam;j++)
						xbee.buf[j]=buf1[j];
					flag_process=1;
					xbee_process();
				}
			}
			}
		if(res>0 & init(buf,res)>0){
			ind1=0;
			while(buf[ind1]!=0x7E){
				buf1[ind]=buf[ind1];
				ind++;
				ind1++;
				}
			printf("buf1= ");
			fflush(stdout);
			for(i=0;i<ind;i++){
				printf("%02x ",buf1[i]);
				fflush(stdout);
				}
			printf("\n");
			fflush(stdout);

		}

		//}
		flag_ser=0;
	}

}

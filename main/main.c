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
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

//#include <mysql/mysql.h>
#include "serial.h"
#include "xbee.h"
#include "cod.h"
#include "ssa.h"
//#include "mysql.h"
#define START 	1
#define END		2

//#define	socket_post1(a,b) socket_postSSL(a,b)

extern void ping_handler();

uint8_t Addr_broadcast[]={0,0,0,0,0,0,0xFF,0xFF};
//##define PASS "822528"
//##define BASE "ssa"
//#MYSQL conexao;
//#MYSQL_RES *resp,*resp1;
//#MYSQL_ROW linhas,linhas1;
//#MYSQL_FIELD *campos;
int ser_receb(void);
void xbee_process(void);
long timeout(struct timeval start);
void discover(void);
void atu_process(void);

void signal_handler_IO (int status);

char buf_rx[100];
uint8_t payload[100];
int fd,flag_s,flag_c;
floatbyte_T float_aux;
uint32_byte_T uint32_aux,uint32_aux1;
//atudisp_T disp;
xbee_t xbee;
floatbyte_T floataux;
int status1=0,res,ind=0,sum=0,tam;
int flag_process=0;
uint8_t buf[200];
uint8_t buf1[200];
char buf2[10][200];
void ser_process();
int flag_ser=0;
uint8_t addraux[8];

#define	socket_post1(a,b) socket_SSL1(a,b,xbee.myaddr)
int main(int argc, char **argv)
{
	struct timeval start, end,start1, end1;
	long mtime=0,mtime1, seconds=0, useconds=0;
	char c;
	int ent,est,i,j;
	float ola;
	char myaddr[20];
	int disp_count=0,disp_aux,flag_status[4],flag;

	fd=serial_init(&signal_handler_IO);
	int type,var;
	xbee.flag_myaddr=1;
	xbee_cmdAT(fd,&xbee,(uint8_t*)"SH");
	//usleep(1000000);
	//xbee_cmdAT(fd,&xbee,(uint8_t*)"SH");

	//printf("ola");
	//fflush(stdout);

	//SSL_load_error_strings();
	//ERR_load_BIO_strings();
	//OpenSSL_add_all_algorithms();


	gettimeofday(&start, NULL);
	gettimeofday(&start1, NULL);
	while(1){
		ser_process();
		xbee_process();
		if(xbee.flag_myaddr==3){


			memset(myaddr,0,sizeof(myaddr));

			xbee_addrstr(xbee.myaddres,xbee.myaddr);
			//strcpy(xbee.myaddr,myaddr);

			sprintf(buf,"act=init&type=5&mac=%s",xbee.myaddr);

			printf("buf=%s\r\n",buf);
			if(socket_post1(buf,&buf1)){
				printf("buf1=%s\r\n",buf1);
                fflush(stdout);
				}

			
			//esp8266_post(buf1,&post_init);
			//printf("buf=%s \r\n",buf1);
			xbee.flag_myaddr=0;
			discover();
			}
		if((timeout(xbee.disc.start)>150000) & (xbee.disc.flag)){
			sprintf(buf,"act=discover&mac=%s&qtd=%d",xbee.myaddr,xbee.disc.qtd);
			for(i=0;i<xbee.disc.qtd;i++){

				sprintf(buf2[1],"&mac%d=%02X%02X%02X%02X%02X%02X%02X%02X"
			,i,xbee.disc.addr[i][0],xbee.disc.addr[i][1],xbee.disc.addr[i][2],xbee.disc.addr[i][3],
			xbee.disc.addr[i][4],xbee.disc.addr[i][5],xbee.disc.addr[i][6],xbee.disc.addr[i][7]);
				strcat(buf,buf2[1]);

				}
			printf("buf=%s \r\n",buf);
			fflush(stdout);
			if(socket_post1(buf,&buf1)){
				printf("buf1=%s\r\n",buf1);
				fflush(stdout);
				}
			//esp8266_post(buf1,&post_discover);
			discover();
			}
		/*if(timeout(start)>5000){
			if(socket_post1("act=teste",&buf)){
				printf("buf=%s\r\n",buf);
				fflush(stdout);
			}
			gettimeofday(&start, NULL);
		}*/
		usleep(1000);
		}

					
	}
long timeout(struct timeval start){
	long mtime=0, seconds=0, useconds=0,timeout;
	struct timeval end;
	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	return mtime;
}
void discover(void){
	xbee_cmdAT(fd,&xbee,"ND");
	xbee.disc.qtd=0;
	gettimeofday(&(xbee.disc.start), NULL);
	//discover.discover_tim_start=TIM2->CNT;
	xbee.disc.flag=1;
}
void xbee_process(void){
	int i,j,k;
	if(flag_process){
		xbee_reciver(&xbee);
		/*printf("packet rec: ");
		fflush(stdout);
		for(i=0;i<xbee.buf[2]+4;i++){
			printf("%02x ",xbee.buf[i]);
			fflush(stdout);
			}
		printf("\n");
		fflush(stdout);*/
		switch(buf1[3]){
			case XBEE_STATUS_SUCESS:
				break;
			case XBEE_CMDATRR:
				if((xbee.buf[15]=='D') & (xbee.buf[16]=='B')){
					memset(buf,0,sizeof(buf));
					sprintf(buf,"act=rssi&rssi=%d&mac=",xbee.buf[18]);
					for(i=0;i<8;i++){
						memset(buf2[0],0,sizeof(buf2[0]));
						sprintf(buf2[0],"%02X",xbee.buf[5+i]);
						strcat(buf,buf2[0]);
						}
					printf("buf=%s\r\n",buf);
					fflush(stdout);
					if(socket_post1(buf,&buf1)){
						printf("buf1=%s\r\n",buf1);
						fflush(stdout);
						atu_process();
						}
					}
				break;
			case XBEE_CMDAT:
				if(xbee.flag_myaddr==1){
					//printf("glag=1\r\n");
					for(i=0;i<4;i++)
						xbee.myaddres[i]=xbee.buf[8+i];
					xbee.flag_myaddr=2;
					xbee_cmdAT(fd,&xbee,(uint8_t*)"SL");
					}
				else if(xbee.flag_myaddr==2){
					//printf("glag=2\r\n");
					for(i=0;i<4;i++)
						xbee.myaddres[4+i]=xbee.buf[8+i];
					xbee.flag_myaddr=3;
					}
				//captura o discover
				if((xbee.buf[5]=='N') & (xbee.buf[6]=='D')){
					for(i=0;i<8;i++){
						xbee.disc.addr[xbee.disc.qtd][i]=xbee.buf[10+i];
						addraux[i]=xbee.buf[10+i];
						}
					//rssi
					xbee_cmdATR(fd,&xbee,"DB",addraux);
					xbee.disc.qtd++;
				}
				break;
			case XBEE_RECEIVE_PACKET:
				switch(xbee.buf[XBEE_PAYLOAD_OFFSET]){
				case SSA_F_OUTP:
					xbee_addrstr(xbee.source_Address,buf1);
					//fflush(stdout);
					//printf("buf1: %s\r\n",buf1);
					sprintf(buf,"act=est&out=%d&est=%d&mac=%s",xbee.buf[XBEE_PAYLOAD_OFFSET+1],xbee.buf[XBEE_PAYLOAD_OFFSET+2],buf1);
					printf("buf=%s\r\n",buf);
					fflush(stdout);
					if(socket_post1(buf,buf1)){
						printf("buf1=%s\r\n",buf1);
						fflush(stdout);
					}
					//esp8266_post(buf,&post_out);
					//esp8266_post("act=teste",&teste);
					//printf("teste1\r\n");
					//printf("buf= ");
					//for(i=0;i<xbee.sizer;i++)
					//	printf("0x%02X ",xbee.buf[3+i]);
					//printf("\r\n");
					break;
					case SSA_F_CORD_ADDR:
						payload[0]=SSA_F_CORD_ADDRP;
						for(i=0;i<8;i++)
							payload[1+i]=xbee.myaddres[i];
						xbee_SendData(fd,&xbee,xbee.source_Address,payload,9);

						//sprintf(buf,"act=init&mac=%s&tipo=");
						//printf("teste\r\n");
					break;
					case SSA_F_ANALOG:
						printf("rec analog\r\n");
						fflush(stdout);
						memset(buf,0,sizeof(buf));
						xbee_addrstr(xbee.source_Address,buf2[0]);
						sprintf(buf,"act=var1&disptype=3&mac=%s&qtd=%d",buf2[0],xbee.buf[XBEE_PAYLOAD_OFFSET+1]);

						for(i=0;i<xbee.buf[XBEE_PAYLOAD_OFFSET+1];i++){
							for(j=0;j<4;j++)
								floataux.byte.b[j]=xbee.buf[XBEE_PAYLOAD_OFFSET+6+j+i*8];
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
							strcat(buf,buf2[0]);

							}
						printf("buf=%s\r\n",buf);
						fflush(stdout);
						if(socket_post1(buf,buf1)){
							printf("buf1=%s\r\n",buf1);
							fflush(stdout);
							atu_process();

							}

						break;
					}
				break;
			}
		flag_process=0;
		}
}
void signal_handler_IO (int status)
{
flag_ser=1;
}
void ser_process(){
	if(flag_ser){
	int i,j;
	res = read(fd,buf,100);
	if (res > 0){
		//printf("\r\n");
		//printf("o=0x%02X res=%d ind=%d status1=%d\r\n",buf[0],res,ind,status1);
		if(buf[0]==0x7e){
			status1=START;
			//ind=0;
			sum=0;
			//printf("start i=%d res=%d\r\n",i,res);
			//fflush(stdout);
			buf1[0]=0x7e;
			ind=1;
			}
		else if(status1==START){
			buf1[ind]=buf[0];
			if(ind>3){
				sum=0;
				for(j=3;j<ind;j++)
					sum+=buf1[j];
				sum&=0xFF;
				sum=0xFF-sum;
				//printf("i=%d sumc=%02X sumi=%02X\r\n",i,sum,buf1[buf1[2]+3]);
				if(buf1[ind]==sum){
					status1=END;
					tam=ind+1;
					for(j=0;j<tam;j++)
						xbee.buf[j]=buf1[j];
					flag_process=1;
					}
				}
			ind++;
			}
		}
	flag_ser=0;
	}

}
//processa resposta da fun��o atu do servidor
void atu_process(void){
	int i,j,k;
	memset(buf,0,sizeof(buf));

	i=0;
	while(buf1[i]<'0' | buf1[i]>'9') i++;
	j=i;
	i=0;
	//printf("teste11\r\n");
	while(buf1[i+j]!=';'){
		buf[i]=buf1[i+j];
		i++;
		}
	int qtd=atoi(buf);
	j+=i+1;
	//printf("buf1[i]=%c buf1[i+1]=%c\r\n",buf[i],buf[i+1]);
	if(qtd>0){
		memset(buf,0,sizeof(buf));
		i=0;
		while(buf1[i+j]!=';'){
			buf[i]=buf1[i+j];
			i++;
			}
		//printf("teste12\r\n");
		//printf("caux=%s \r\n",buf);
		memset(addraux,0,sizeof(addraux));
		sscanf(buf,"%02X%02X%02X%02X%02X%02X%02X%02X",&addraux[0],&addraux[1],&addraux[2],&addraux[3],&addraux[4],&addraux[5],&addraux[6],&addraux[7]);
		//printf("teste13 qtd=%d\r\n",qtd);
		//xbee_addrstr(addraux,buf2[0]);
		//printf("addraux=%s\r\n",buf2[0]);
		/*printf("addr: ") ;
		for(i=0;i<8;i++)
			printf("%02X ",addraux[i]);
		printf("\r\n");*/
		i=0;
		while(buf1[i+j]!=';') i++;
		j+=i+1;
		memset(payload,0,sizeof(payload));
		payload[0]=SSA_F_ANALOG_TEMPO;
		payload[1]=qtd;
		//printf("buf1[i]=%c buf1[i+1]=%c buf1[i+2]=%c buf1[j-1]=%c\r\n",buf[j],buf[j+1],buf[j+2],j,buf[j-1]);
		for(k=0;k<qtd;k++){

			//variavel
			memset(buf,0,sizeof(buf));
			i=0;
			while(buf1[i+j]!=';'){
				buf[i]=buf1[i+j];
				i++;
				}
			//printf("teste14\r\n");
			//printf("var=%s\r\n",buf);
			payload[2+k*3]=atoi(buf);
			//tempo
			memset(buf,0,sizeof(buf));
			//printf("teste15\r\n");
			j+=i+1;
			//printf("buf1[i]=%c buf1[i+1]=%c buf1[i+2]=%c buf1[%d]=%c\r\n",buf1[i],buf1[i+1],buf1[i+2],j,buf1[j]);
			i=0;
			while(buf1[i+j]!=';'){
				buf[i]=buf1[i+j];
				i++;
				}
			payload[3+k*3]=atoi(buf);
			//printf("tempo=%s\r\n",buf);
			//medias
			memset(buf,0,sizeof(buf));
			//printf("teste15\r\n");
			j+=i+1;
			//printf("buf1[i]=%c buf1[i+1]=%c buf1[i+2]=%c buf1[%d]=%c\r\n",buf1[i],buf1[i+1],buf1[i+2],j,buf1[j]);
			i=0;
			while(buf1[i+j]!=';'){
				buf[i]=buf1[i+j];
				i++;
				}
			payload[4+k*3]=atoi(buf);
			//printf("media=%s\r\n",buf);
			j+=i+1;
			}
		/*printf("out: ");
		for(i=0;i<2+3*qtd;i++)
			printf("%02X ",payload[i]);
		printf("\r\n");*/

		xbee_SendData(fd,&xbee,addraux,payload,2+3*qtd);
		//xbee_cmdATR(fd,&xbee,"DB",xbee.source_Address);
		}
}

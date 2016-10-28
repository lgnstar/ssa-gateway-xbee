/*
 * xbee.c
 *
 *  Created on: 14/08/2015
 *      Author: leandro
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "xbee.h"
#include "serial.h"
void xbee_get_address(xbee_t *xbee);
void xbee_reciver(xbee_t *xbee){

	xbee->type=xbee->buf[3];
	xbee_get_address(xbee);
}
void xbee_get_address(xbee_t *xbee){
	int i;
	for(i=0;i<8;i++){
		xbee->source_Address[i]=xbee->buf[4+i];
		//printf("i=%d %02X\r\n",i,xbee->buf[4+i]);
		}
}
void xbee_packet_data(xbee_t *xbee,uint8_t *address,uint8_t *data, uint8_t size){
	int i;
	for(i=0;i<8;i++)
			xbee->payload[5+i]=address[i];
	xbee->payload[13]=0xFF;
	xbee->payload[14]=0xFE;
	xbee->payload[15]=0x00;
	xbee->payload[16]=0x00;
	for(i=17;i<(size+17);i++){
			xbee->payload[i]=data[i-17];
		}
	xbee->size=size+12;
}
uint8_t xbee_checksum(xbee_t *xbee){
	int sum=0,i;
	for(i=3;i<xbee->size+3;i++)
		sum+=xbee->payload[i];
	sum&=0xFF;
	sum=0xFF-sum;
	return sum;
}
void xbee_packet(xbee_t *xbee,uint8_t type){
	int i;
	xbee->size+=2;
	xbee->payload[0]=0x7e;
	xbee->payload[1]=0;
	xbee->payload[2]=xbee->size;
	//tipo
	xbee->payload[3]=type;
	//id
	xbee->payload[4]=0x10;
	//address

	xbee->payload[xbee->size+3]=xbee_checksum(xbee);
}
void xbee_Send(xbee_t *xbee,int fd){
	int i;
//	printf("ola %d\r\n",xbee->size);
	write(fd,xbee->payload,xbee->size+4);
//	sync();
	//fflush(NULL);
//	printf("ola1\r\n");

	//fsync(fd);
	//fflush(NULL);
	//fflush(fd);
}

void xbee_SendData(int fd,xbee_t *xbee,uint8_t *address,uint8_t *data,uint8_t size){
	//xbee_data(xbee,data,size);
	xbee_packet_data(xbee,address,data,size);
	//xbee_address(xbee,address);
	xbee_packet(xbee,XBEE_TRANSMIT_REQUEST);
	xbee_Send(xbee,fd);
}

void xbee_cmdAT(int fd,xbee_t *xbee,uint8_t *str){

	xbee->payload[5]=str[0];
	xbee->payload[6]=str[1];
	xbee->size=2;
	xbee_packet(xbee,XBEE_CMD);
	xbee_Send(xbee,fd);
}
void xbee_cmdATR(int fd,xbee_t *xbee,uint8_t *str,uint8_t *addr){
	int i;
	//addr
	//printf("str= %02X %02X\r\n addr= ",str[0],str[1]);
	for(i=0;i<8;i++){
		xbee->payload[5+i]=addr[i];
		//printf("%02X ",addr[i]);
		}
	//16bits addr
	xbee->payload[13]=0xFF;
	xbee->payload[14]=0xFE;
	//cmd options
	xbee->payload[15]=0x02;
	//cmd
	xbee->payload[16]=str[0];
	xbee->payload[17]=str[1];
	xbee->size=13;
	xbee_packet(xbee,XBEE_CMDATRT);
	//printf("\r\nsend: ");
	//for(i=0;i<xbee->size+4;i++)
		//printf("%02X ",xbee->payload[i]);
	//printf("\r\n");
	xbee_Send(xbee,fd);
}
void xbee_addrstr(uint8_t *addr,uint8_t *str){
	sprintf((char*)str,"%02X%02X%02X%02X%02X%02X%02X%02X",addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],addr[6],addr[7]);
}

/*
 * xbee.h
 *
 *  Created on: 14/08/2015
 *      Author: leandro
 */

#ifndef XBEE_H_
#define XBEE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>
#include <sys/time.h>

#define XBEE_RECEIVE_PACKET 	0x90
#define XBEE_TRANSMIT_REQUEST	0x10
#define XBEE_CMD				0x08
#define XBEE_CMDAT				0x88
#define XBEE_CMDATRT			0x17
#define XBEE_CMDATRR			0x97
#define XBEE_STATUS				0x8B
#define XBEE_PAYLOAD_OFFSET		0x0F
#define	XBEE_STATUS_SUCESS		0x08

typedef union{
	float val;
	struct{
		char b[4];
    	}byte;
}floatbyte_T;

typedef struct{
	int addr[10][8];
	int qtd;
	struct timeval start;
	int flag;
}discoverT;

typedef struct{
	uint8_t type;
	char myaddr[20];
	uint8_t myaddres[8];
	uint8_t Address[8];
	uint8_t addr_cord[8];
	uint8_t source_Address[8];
	uint8_t payload[200];
	uint8_t buf[200];
	uint8_t size,id,start;
	uint8_t sizer;
	uint8_t flag_myaddr;
	discoverT disc;
}xbee_t;

typedef union{
	uint32_t val;
	struct{
		char b[4];
    	}byte;
}uint32_byte_T;

void xbee_reciver(xbee_t *xbee);
void xbee_SendData(int fd,xbee_t *xbee,uint8_t *address,uint8_t *data,uint8_t size);
void xbee_PedRSSI(int fd,xbee_t *xbee,uint8_t *addr);
void xbee_cmdAT(int fd,xbee_t *xbee,uint8_t *str);
void xbee_cmdATR(int fd,xbee_t *xbee,uint8_t *str,uint8_t *addr);
void xbee_addrstr(uint8_t *addr,uint8_t *str);
#endif /* XBEE_H_ */

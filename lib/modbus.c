#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>

/* global data */

int modbus(char *ip,uint8_t unit,uint16_t reg){
	int s;
  	int i;
  	struct sockaddr_in server;
  	fd_set fds;
  	struct timeval tv;
  	unsigned char obuf[261];
  	unsigned char ibuf[261];

  	/* establish connection to gateway on ASA standard port 502 */
  	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  	server.sin_family = AF_INET;
  	server.sin_port = htons(502); /* ASA standard port */
  	server.sin_addr.s_addr = inet_addr(ip);

  	i = connect(s, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
  	if (i<0){
    		printf("connect - error %d\n",i);
    		close(s);
    		return -1;
  		}

  	FD_ZERO(&fds);
  	tv.tv_sec = 5;
  	tv.tv_usec = 0;

  	/* check ready to send */
  	FD_SET(s, &fds);
  	i = select(32, NULL, &fds, NULL, &tv);
  	if (i<=0){
    		printf("select - error %d\n",i);
    		close(s);
    		return -1;
  		}

  	/* build MODBUS request */
  	for (i=0;i<5;i++) obuf[i] = 0;
  	obuf[5] = 6; //qtd
  	obuf[6] = unit;
  	obuf[7] = 4;
  	obuf[8] = reg  >> 8;
  	obuf[9] = reg & 0xff;
  	obuf[10] = 0;
  	obuf[11] = 1;
  	/* send request */
  	i = send(s, obuf, 12, 0);
  	if (i<12){
		close(s);
    		return -1;
    		printf("failed to send all 12 chars\n");
  		}

  	/* wait for response */
  	FD_SET(s, &fds);
  	i = select(32, &fds, NULL, NULL, &tv);
  	if (i<=0){
    		printf("no TCP response received\n");
    		close(s);
    		return 1;
  		}

  	/* receive and analyze response */
  	i = recv(s, ibuf, 261, 0);
  	if (i<9){
    		if (i==0)
      			printf("unexpected close of connection at remote end\n");
    		else
      			printf("response was too short - %d chars\n", i);
  		}
  	else if (ibuf[7] & 0x80)	
    		printf("MODBUS exception response - type %d\n", ibuf[8]);
  	else{
		close(s);
		return (ibuf[9]<<8) + ibuf[10];
  		}

  	/* close down connection */
  	close(s);
	return -1;
	}


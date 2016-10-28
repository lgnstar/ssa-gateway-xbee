#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <resolv.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <linux/sockios.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "cod.h"

#define BUF_MAX 4096

#define PACKETSIZE  64
struct packet
{
    struct icmphdr hdr;
    char msg[PACKETSIZE-sizeof(struct icmphdr)];
};


typedef union{
	float f;
	struct{
		char b[4];
		}byte;
	}float_T;

	
int socket_var(char *address, int var,float *f)
{
	int out=0;
	float_T aux_float;
	uint8_t ibuf[20],j;
	uint8_t obuf[20];
  	int Meusocket, numbytes,i;
	char buf[20];
	struct sockaddr_in seu_endereco;
	fd_set readfds,masterfds;
  	struct timeval tv;

	//FD_ZERO(&fds);
  	tv.tv_sec = 3;
  	tv.tv_usec = 0;
	

	if ((Meusocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		close(Meusocket);
		return 0;
	}
	FD_ZERO(&masterfds);
   	FD_SET(Meusocket, &masterfds);
	
	memcpy(&readfds, &masterfds, sizeof(fd_set));
	
	
	
	seu_endereco.sin_family = AF_INET;
	seu_endereco.sin_port = htons(9500);
	seu_endereco.sin_addr.s_addr = inet_addr(address);
	bzero(&(seu_endereco.sin_zero), 8);

	if (connect(Meusocket,(struct sockaddr *)&seu_endereco, sizeof(struct sockaddr)) ==-1) 
	{
		close(Meusocket);
		return 0;
	}
	obuf[0]=FUNC_PEDVAR;
	obuf[1]=var;
	/*FD_SET(Meusocket, &fds);
	printf("enviar\n");
  	i = select(32, NULL, &fds, NULL, &tv);
	if (i<=0){
    		printf("select - error %d\n",i);
    		close(Meusocket);
    		return 0;
  		}*/
	
	send(Meusocket, obuf, 2, 0);

	//printf("recebe\n");
	/*FD_SET(Meusocket, &fds);
  	i = select(32, NULL, &fds, NULL, &tv);
	if (i<=0){
    		printf("select - error %d\n",i);
    		close(Meusocket);
    		return 0;
  		}*/
	
	//printf("i=%d\n",i);
	//for(j=0;j<(i);j++)
	//	printf("bufi[%d]=%d\n",j,ibuf[j]);
	//printf("\n");
	if (select(Meusocket+1, &readfds, NULL, NULL, &tv) < 0){
     		perror("on select");
     		exit(0);
   		}
	if (FD_ISSET(Meusocket, &readfds)){
     		i = recv(Meusocket, ibuf,20, 0);
		if(i!=0){
			out=1;
			for(i=0;i<4;i++)
				aux_float.byte.b[i]=ibuf[1+i];
			//printf("f=%f\n",aux_float.f);
			}
   		}
   	else{
		close(Meusocket);
		//printf("timeout\n");
     // the socket timedout
  		return 0;
		
		}
	
 	close(Meusocket);
	*f=aux_float.f;
	return 1;

}
int socket_send(char *buf, int tam, char *ip, int porta)
{
	int out=0;
	uint8_t ibuf[100],j;
	char obuf[100];
  	int Meusocket, numbytes,i;
	
	struct sockaddr_in seu_endereco;
	fd_set readfds,masterfds;
  	struct timeval tv;

	//FD_ZERO(&fds);
  	tv.tv_sec = 3;
  	tv.tv_usec = 0;
	

	if ((Meusocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		close(Meusocket);
		return 0;
	}
	//FD_ZERO(&masterfds);
   	//FD_SET(Meusocket, &masterfds);
	
	//memcpy(&readfds, &masterfds, sizeof(fd_set));
	
	
	
	seu_endereco.sin_family = AF_INET;
	seu_endereco.sin_port = htons(porta);
	seu_endereco.sin_addr.s_addr = inet_addr(ip);
	bzero(&(seu_endereco.sin_zero), 8);

	if (connect(Meusocket,(struct sockaddr *)&seu_endereco, sizeof(struct sockaddr)) ==-1) 
	{
		close(Meusocket);
		return 0;
	}
	//printf("f=%f\n",_val);
	//sprintf(obuf,"GET /update?key=%s&field%d=%2.2f \r\n",key,field,valor);
	//printf("\"%s\" tam=%d",obuf,strlen(obuf));
	
	
	send(Meusocket, buf, tam, 0);

	/*if (select(Meusocket+1, &readfds, NULL, NULL, &tv) < 0){
     		perror("on select");
     		exit(0);
   		}
	if (FD_ISSET(Meusocket, &readfds)){
     		i = recv(Meusocket, ibuf,200, 0);
		if(i!=0){
			printf("%s",ibuf);
			}
   		}*/

	
 	close(Meusocket);
	//f=aux_float.f;
	return 1;

}
int socket_thingspeak(char *key, int field, float valor)
{
	int out=0;
	printf("f=%f\n",valor);
	uint8_t ibuf[100],j;
	char obuf[100];
  	int Meusocket, numbytes,i;
	char buf[20];
	struct sockaddr_in seu_endereco;
	fd_set readfds,masterfds;
  	struct timeval tv;

	//FD_ZERO(&fds);
  	tv.tv_sec = 3;
  	tv.tv_usec = 0;
	

	if ((Meusocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		close(Meusocket);
		return 0;
	}
	FD_ZERO(&masterfds);
   	FD_SET(Meusocket, &masterfds);
	
	memcpy(&readfds, &masterfds, sizeof(fd_set));
	
	
	
	seu_endereco.sin_family = AF_INET;
	seu_endereco.sin_port = htons(80);
	seu_endereco.sin_addr.s_addr = inet_addr("54.164.214.198");
	bzero(&(seu_endereco.sin_zero), 8);

	if (connect(Meusocket,(struct sockaddr *)&seu_endereco, sizeof(struct sockaddr)) ==-1) 
	{
		close(Meusocket);
		return 0;
	}
	//printf("f=%f\n",_val);
	sprintf(obuf,"GET /update?key=%s&field%d=%2.2f \r\n",key,field,valor);
	//printf("\"%s\" tam=%d",obuf,strlen(obuf));
	
	
	send(Meusocket, obuf, strlen(obuf), 0);

	/*if (select(Meusocket+1, &readfds, NULL, NULL, &tv) < 0){
     		perror("on select");
     		exit(0);
   		}
	if (FD_ISSET(Meusocket, &readfds)){
     		i = recv(Meusocket, ibuf,200, 0);
		if(i!=0){
			printf("%s",ibuf);
			}
   		}*/

	
 	close(Meusocket);
	//f=aux_float.f;
	return 1;

}
int socket_post(char *post, char *resp){
	int s;
  	int i;
  	struct sockaddr_in server;
  	fd_set fds;
  	struct timeval tv;
  	unsigned char obuf[361];
  	unsigned char ibuf[561];
  	char out[500];

  	/* establish connection to gateway on ASA standard port 502 */
  	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  	server.sin_family = AF_INET;
  	server.sin_port = htons(80); /* ASA standard port */
  	server.sin_addr.s_addr = inet_addr("166.62.101.58");

  	i = connect(s, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
  	if (i<0){
    		printf("connect - error %d\n",i);
    		close(s);
    		return 0;
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
    		return 0;
  		}

  	sprintf(obuf,"POST /ssa/teste/teste.php  HTTP/1.0\r\n");
	sprintf(obuf,"%sHost: lronetto.com\r\n",obuf);
	sprintf(obuf,"%sContent-Type: application/x-www-form-urlencoded\r\n",obuf);
	sprintf(obuf,"%sContent-Length: %d \r\n\r\n",obuf,strlen(post));
	sprintf(obuf,"%s%s\r\n",obuf,post);
  	/* send request */
  	i = send(s, obuf, strlen(obuf)+1, 0);
  	if (i<(strlen(obuf)+1)){
		close(s);
    		return 0;
    		printf("failed to send all 12 chars\n");
  		}

  	/* wait for response */
  	FD_SET(s, &fds);
  	tv.tv_sec = 3;
  	tv.tv_usec = 0;
  	i = select(32, &fds, NULL, NULL, &tv);
  	if (i<=0){
    		printf("no TCP response received\n");
    		close(s);
    		return 0;
  		}

  	/* receive and analyze response */
  	i = recv(s, ibuf, 261, 0);
  	printf("i=%d\r\n",i);
	fflush(stdout);
  	//printf("buf=%s\r\n",ibuf);
  	int j,bn=0,offset=0,cnt=0;
	printf("\r\n");
	fflush(stdout);
	cnt=0;
	bn=0;
  	for(j=0;j<i;j++){
		printf("%c",ibuf[j]);
		fflush(stdout);

  		if(ibuf[j]=='\n') bn++;
  		if(bn==9){
  			offset=j+1;
  			out[cnt++]=ibuf[j+1];
  			//break;
  		}
  	}
	printf("\r\n cnt=%d bn=%d\r\n",cnt,bn);
  	out[cnt-1]='\0';
  	//memcpy(resp,out,strlen(out));
  	sprintf(resp,"%s",out);
  	//printf("buf=%s\r\n",out);
	fflush(stdout);
  	//printf("bn=%d obuf[j]=%c %c %c\r\n",bn,ibuf[offset],ibuf[offset+1],ibuf[offset+2]);
  	/* close down connection */
  	close(s);
	return 1;
	}

int get_ip(char *host,char *ip)
{
  struct hostent *hent;
  int iplen = 15; //XXX.XXX.XXX.XXX
  //char *ip = (char *)malloc(iplen+1);
  memset(ip, 0, iplen+1);
  if((hent = gethostbyname(host)) == NULL)
  {
    printf("Can't get IP");
    return 0;
  }
  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
  {
    printf("Can't resolve host");
    return 0;
  }
  //sprintf()
  return 1;
}
int socket_open(int port){
	char ip[20];
	int Meusocket;
	struct sockaddr_in *seu_endereco;
	//printf("teste1\r\n");
	if ((Meusocket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0 )
	{
		close(Meusocket);
		return 0;
	}
	//printf("teste2\r\n");
	seu_endereco=(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	seu_endereco->sin_family = AF_INET;
	seu_endereco->sin_port = htons(port);
	//printf("teste3\r\n");
	if(!get_ip("lronetto.com",ip))
		return 0;
	//printf("teste4\r\n");
	int f=inet_pton(AF_INET, ip, (void *)(&(seu_endereco->sin_addr.s_addr)));
	if(f<0){
		printf("erro addr");
		free(seu_endereco);
		return 0;
		}
	if (connect(Meusocket,(struct sockaddr *)seu_endereco, sizeof(struct sockaddr))<0)
	{
		printf("connect error \r\n");
		free(seu_endereco);
		close(Meusocket);
		return 0;
	}
	free(seu_endereco);
	return Meusocket;
}
int socket_post1(char *post,char *buf)
{
	int out=0;
	//printf("f=%f\n",valor);
	uint8_t ibuf[300],j;
	char obuf[1000],ibuf1[500];
  	char ip[20];
	int sock, numbytes,i;
	struct sockaddr_in *seu_endereco;
	//printf("teste1\r\n");
	if((sock=socket_open(80))==0){
		printf("open error\r\n");
		return 0;
	}
	//printf("f=%f\n",_val);
	sprintf(obuf,	"POST /ssa/teste/teste.php  HTTP/1.0\r\n"
					"Host: lronetto.com\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Content-Length: %d \r\n\r\n"
					"%s\r\n",
					strlen(post),post);

	//printf("teste5\r\n");
	//printf("out %d: %s\r\n",strlen(obuf),obuf);
	//fflush(stdout);
	if(send(sock, obuf, strlen(obuf), 0)<0){
		printf("send error\r\n");
		//free(seu_endereco);
		close(sock);
		return 0;
	}

	memset(ibuf,0,sizeof(ibuf));
	
	printf("teste6\r\n");

	char * htmlcontent;
	int htmlstart = 0,tmpres;

	memset(ibuf,0,sizeof(ibuf));
	memset(ibuf1,0,sizeof(ibuf1));
	if((tmpres=recv(sock, ibuf, BUFSIZ, 0))<0){
		printf("recv error\r\n");
		//free(seu_endereco);
		close(sock);
		return 0;
	}

	while(tmpres>0){
		strcat(ibuf1,ibuf);
		//printf(ibuf);
		memset(ibuf, '\0', sizeof(ibuf));
		tmpres=recv(sock, ibuf, BUFSIZ, 0);
		}
	htmlcontent=strstr(ibuf1,"\r\n\r\n")+8;
	if(htmlcontent==NULL){
		//free(seu_endereco);
		close(sock);
		return 0;
	}
	sprintf(buf,htmlcontent);


	//free(seu_endereco);
 	close(sock);

 	printf("teste7\r\n");
	return 1;

}
int socket_postSSL(char *post,char *buf)
{
	 int sd;
    struct hostent *host;
    struct sockaddr_in addr;
    BIO *outbio = NULL;
    SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL *ssl;
    char *req;
    int req_len;
    char hostname[] ="lronetto.com";
    char certs[] = "/etc/ssl/certs/ca-certificates.crt";
    int port = 443;
    int bytes,rb;
char *pointer;
	uint8_t obuf[200];
	uint8_t ibuf1[200];
	uint8_t ibuf[200];
    // added this to test
       char           dest_url[] = "https://lronetto.com";
       BIO              *certbio = NULL;
       X509                *cert = NULL;
       X509_NAME       *certname = NULL;
    BIO *outbio2 = NULL;



    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    outbio    = BIO_new(BIO_s_file());
    outbio    = BIO_new_fp(stdout, BIO_NOCLOSE);
      
    if(SSL_library_init() < 0){
        BIO_printf(outbio, "Could not initialize the OpenSSL library !\n");
        //SSL_free(ssl);
		//close(sd);
		//SSL_CTX_free(ctx);
    	return 0;
	}

    method = SSLv23_client_method();
    ctx = SSL_CTX_new(method);
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

    if((sd=socket_open(443))==0){
    	printf("open error\r\n");
    	SSL_CTX_free(ctx);
    	return 0;
    	}

    if((ssl = SSL_new(ctx))==NULL){
    	printf("SSL_new error\r\n");
    	SSL_CTX_free(ctx);
    	close(sd);
    	return 0;
    	}
    if(!SSL_set_fd(ssl, sd)){
    	printf("SSL_set_fd error\r\n");
    	SSL_free(ssl);
    	SSL_CTX_free(ctx);
    	close(sd);
    	return 0;
    	}
    

   if( SSL_connect(ssl)<1){
	   printf("SSL_connect error\r\n");
	   SSL_free(ssl);
	   SSL_CTX_free(ctx);
	   close(sd);
	   return 0;
   	   }
	 sprintf(obuf,   "POST /ssa/teste/teste.php  HTTP/1.0\r\n"
                                        "Host: lronetto.com\r\n"
                                        "Content-Type: application/x-www-form-urlencoded\r\n"
                                        "Content-Length: %d \r\n\r\n"
                                        "%s\r\n",
                                        strlen(post),post);
    if((rb=SSL_write(ssl, obuf, sizeof(obuf)))<1){
    	printf("SSL_write error\r\n");
    	SSL_free(ssl);
    	SSL_CTX_free(ctx);
    	close(sd);
    	return 0;
    	}

    printf("teste7\r\n");

    memset(ibuf, '\0', sizeof(ibuf));
 	memset(ibuf1, '\0', sizeof(ibuf1));
    int error;
    if((bytes = SSL_read(ssl, ibuf, sizeof(ibuf)))<1){
    	error=SSL_get_error(ssl,bytes);
    	//printf(ibuf);
    	switch(error){
    		case SSL_ERROR_NONE:
    			printf("SSL_ERROR_NONE\r\n");
    			break;
    		case SSL_ERROR_WANT_CONNECT:
    			printf("SSL_ERROR_WANT_CONNECT\r\n");
    			break;
    		case SSL_ERROR_WANT_ACCEPT:
    			printf("SSL_ERROR_WANT_ACCEPT\r\n");
    			break;
    		case SSL_ERROR_WANT_X509_LOOKUP:
    			printf("SSL_ERROR_WANT_X509_LOOKUP\r\n");
    			break;
    		//case SSL_ERROR_WANT_ASYNC:
    		//	printf("SSL_ERROR_WANT_ASYNC\r\n");
    		//	break;
    		//case SSL_ERROR_WANT_ASYNC_JOB:
    		//	printf("SSL_ERROR_WANT_ASYNC_JOB\r\n");
    		//	break;
    		case SSL_ERROR_SYSCALL:
    			printf("SSL_ERROR_SYSCALL\r\n");
    			break;
    		case SSL_ERROR_SSL:
    			printf("SSL_ERROR_SSL\r\n");
    			break;
    		case SSL_ERROR_ZERO_RETURN:
    			printf("SSL_ERROR_ZERO_RETURN\r\n");
    			break;
    	}
    	printf("SSL_read error %d\r\n",error);
    	SSL_shutdown (ssl);
    	SSL_free(ssl);
    	SSL_CTX_free(ctx);
    	close(sd);
    	return 0;
		}
    while(bytes > 0){
        //write(STDOUT_FILENO, bufi, bytes);
    	strcat(ibuf1,ibuf);
    	memset(ibuf, '\0', sizeof(ibuf));
        bytes = SSL_read(ssl, ibuf, sizeof(ibuf));
    	}
    pointer=strstr(ibuf1,"\r\n\r\n")+8;
    if(pointer==NULL){
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		close(sd);
		return 0;
    	}
	sprintf(buf,pointer);
	//free(pointer);
	//SSL_shutdown (ssl);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	close(sd);


return 1;
}
int socket_modbus(char *ip,uint8_t unit,uint16_t reg){
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
    		return -1;
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
		int j;
		for(j=0;j<i;j++) printf("j=%d =%d\n",j,ibuf[j]);
		close(s);
		int aux=((ibuf[9]<<8) + ibuf[10]);
		if(aux & 0x8000){
			aux&= 0x7FFF;
			return aux*(-1);
			}
		else return aux;
  		}

  	/* close down connection */
  	close(s);
	return 1;
	}

static char *ethernet_mactoa(struct sockaddr *addr) { 
static char buff[256]; 
	
	unsigned char *ptr = (unsigned char *) addr->sa_data;

	sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", 
		(ptr[0] & 0xff), (ptr[1] & 0xff), (ptr[2] & 0xff), 
		(ptr[3] & 0xff), (ptr[4] & 0xff), (ptr[5] & 0xff)); 

return (buff); 

} 


int socket_mac(char *mac,char *ip) {
	int s;
	struct arpreq areq;
	struct sockaddr_in *sin;
	struct in_addr ipaddr;
	
	/* Get an internet domain socket. 
	*/
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(0);
	}
	
	/* Make the ARP request. 
	*/
	memset(&areq, 0, sizeof(areq));
	sin = (struct sockaddr_in *) &areq.arp_pa;
	sin->sin_family = AF_INET;

	if (inet_aton(ip, &ipaddr) == 0) {
		exit(0);
	}
	
	sin->sin_addr = ipaddr;
	sin = (struct sockaddr_in *) &areq.arp_ha;
	sin->sin_family = ARPHRD_ETHER;
	
	strncpy(areq.arp_dev, "eth0", 15);
	
	if (ioctl(s, SIOCGARP, (caddr_t) &areq) == -1) {
		perror("-- Error: unable to make ARP request, error");
		exit(0);
	}
	close(s);
	sprintf(mac,"%s",ethernet_mactoa(&areq.arp_ha));
	return 1;
}

int ping(char *ip)
{
	struct in_addr dst;
    struct icmphdr icmp_hdr;
    struct sockaddr_in addr;
	inet_aton(ip, &dst);
    int sequence = 0;
    int sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return ;
    }

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr = dst;

    memset(&icmp_hdr, 0, sizeof icmp_hdr);
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.un.echo.id = 1234;//arbitrary id

    for (;;) {
        unsigned char data[2048];
        int rc;
        struct timeval timeout = {0, 200000}; //wait max 3 seconds for a reply
        fd_set read_set;
        socklen_t slen;
        struct icmphdr rcv_hdr;

        icmp_hdr.un.echo.sequence = sequence++;
        memcpy(data, &icmp_hdr, sizeof icmp_hdr);
        memcpy(data + sizeof icmp_hdr, "hello", 5); //icmp payload
        rc = sendto(sock, data, sizeof icmp_hdr + 5,
                        0, (struct sockaddr*)&addr, sizeof addr);
        if (rc <= 0) {
            perror("Sendto");
            break;
        }
        //puts("Sent ICMP\n");

        memset(&read_set, 0, sizeof read_set);
        FD_SET(sock, &read_set);

        //wait for a reply with a timeout
        rc = select(sock + 1, &read_set, NULL, NULL, &timeout);
        if (rc == 0) {
            close(sock);
	return 0;//puts("Got no reply\n");
            continue;
        } else if (rc < 0) {
            perror("Select");
            break;
        }

        //we don't care about the sender address in this example..
        slen = 0;
        rc = recvfrom(sock, data, sizeof data, 0, NULL, &slen);
        if (rc <= 0) {
            perror("recvfrom");
            break;
        } else if (rc < sizeof rcv_hdr) {
            printf("Error, got short ICMP packet, %d bytes\n", rc);
            break;
        }
        memcpy(&rcv_hdr, data, sizeof rcv_hdr);
        if (rcv_hdr.type == ICMP_ECHOREPLY) {
		close(sock);
		return 1;
            //printf("ICMP Reply, id=0x%x, sequence =  0x%x\n",icmp_hdr.un.echo.id, icmp_hdr.un.echo.sequence);
        } else {
            printf("Got ICMP packet with type 0x%x ?!?\n", rcv_hdr.type);
        }
    }
}



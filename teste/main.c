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
#include "socket.h"

int main(){

uint8_t buf[100],buf1[100];
sprintf(buf,"act=teste");
while(1){

socket_postSSL(buf,buf1);
printf("%s\r\n",buf1);

sleep(1);
}
}

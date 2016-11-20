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

int total=0;
int total_certo=0;

#define DEBUG 1

int socket_SSL(char *post,char *buf,char *addr){
	BIO * bio,*out = NULL;
	SSL * ssl;
	SSL_CTX * ctx;
	char bufaux[100];
	char r[1024],post_perca[300];
	char ibuf1[1024];
	int bytes;
	char *pointer;
	char obuf[1000];
	int i=0,j;
	//printf("ssl\n");

	SSL_library_init();
	ERR_load_BIO_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();

	/* Set up the SSL context */
	ctx = SSL_CTX_new(SSLv23_client_method());
	/* Load the trust store */
	//printf("teste1\r\n");
	if(! SSL_CTX_load_verify_locations(ctx, "/home/TrustStore.pem", NULL))
	{
		//printf("teste2\r\n");
		fprintf(stderr, "Error loading trust store\n");
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		//ERR_free_strings();
		//EVP_cleanup();
		return 0;
	}
	//printf("teste2\r\n");
	bio = BIO_new_ssl_connect(ctx);
	//printf("teste3\r\n");
	BIO_get_ssl(bio, & ssl);
	//printf("teste4\r\n");
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	//printf("teste5\r\n");

	/* Create and setup the connection */

	BIO_set_conn_hostname(bio, "lronetto.com:443");

	//printf("teste6\r\n");
	//out = BIO_new_fp(stdout, BIO_NOCLOSE);
	if(BIO_do_connect(bio) <= 0){
		fprintf(stderr, "Error attempting to connect\n");
		ERR_print_errors_fp(stderr);
		BIO_free_all(bio);
		SSL_CTX_free(ctx);
		//ERR_free_strings();
		//EVP_cleanup();
		return 0;

	}

	/* Check the certificate */

	//printf("teste7\r\n");
	if(SSL_get_verify_result(ssl) != X509_V_OK)

	{
		fprintf(stderr, "Certificate verification error: %i\n", SSL_get_verify_result(ssl));
		BIO_free_all(bio);
		SSL_CTX_free(ctx);
		//ERR_free_strings();
		//EVP_cleanup();
		return 0;
	}

	//printf("teste8\r\n");

	fprintf(stderr,"post=\"%s\"\n",post);
	//fflush(stdout);

	memset(obuf,0,sizeof(obuf));
	sprintf(obuf,   "POST /ssa/scripts/gate_xbee.php  HTTP/1.0\r\n"
					"Host: lronetto.com\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Content-Length: %d \r\n"
					"Connection: Close\r\n\r\n"
					"%s\r\n",
					strlen(post),post);

	//printf("obuf=\"%s\"",obuf);
	//fflush(stdout);

	BIO_puts(bio, obuf);//, strlen(obuf));
	memset(ibuf1,0,sizeof(ibuf1));
	memset(r,0,sizeof(r));
	//printf("teste9\r\n");
	bytes = BIO_read(bio, r, BUFSIZ);
	while(bytes > 0){
		//printf(r);
		strcat(ibuf1,r);

		bytes = BIO_read(bio, r, BUFSIZ);
		}

	//printf("socket1 len=%d\r\n",strlen(ibuf1));
	//fflush(stdout);
	//printf("socket1\n");
	//fflush(stdout);

	BIO_free_all(bio);
	//BIO_free()
	//printf("socket11\n");
	//fflush(stdout);

	SSL_CTX_free(ctx);
	//printf("socket12\n");
	//fflush(stdout);

	//BIO_ssl_shutdown(bio);
	//printf("socket13\n");
	//fflush(stdout);

	//SSL_shutdown(ssl);

	//ERR_free_strings();
	//fflush(stdout);
	//fprintf(stderr,"socket13\n");


	//EVP_cleanup();
	//printf("socket2\n");
	//fflush(stdout);
	 if(strlen(ibuf1)==0){
               // printf("slrlen=0\r\n");
                //fflush(stdout);
                return 0;
                }
	//printf("teste11\r\n");
	//fflush(stdout);
	//printf("socket3\n");
	//fflush(stdout);
	if(total==2){
		//printf("ibuf1=\"%s\"",ibuf1);
		//fflush(stdout);
	}
	//fflush(stdout);
	int len=strlen(ibuf1);
	i=0;
	while(!(ibuf1[i]=='$' & ibuf1[i+1]=='$') & i<(len)) i++;
	if(i>=(len)){
		//printf("erro n t $$\r\n");
		//fflush(stdout);
		return 0;
	}
	///printf("socket4\n");
	//fflush(stdout);
	memset(obuf,0,sizeof(obuf));
	j=0;
	i++;
	while(ibuf1[i]!='&' && i<len){
		obuf[j]=ibuf1[i];
		i++;
		j++;
	}
	if(i>=len){
		//printf("sem &\r\n");
		//fflush(stdout);
		return 0;
		}
	sprintf(buf,obuf);

	total_certo++;
	//printf("socket5\n");
	//fflush(stdout);
	//printf("socket6\n");
	//fflush(stdout);
	 return 1;
}
int socket_SSL1(char *post,char *buf,char *addr){
	int tam=0;
	BIO * bio;
	SSL_CTX * ctx;
	SSL * ssl;
	char obuf[300],inbuf;
	char r[1024];
	/* Initializing OpenSSL */
	//printf("ssl1\n");
	SSL_library_init();
	ERR_load_BIO_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	//printf("ssl2\n");

	ctx = SSL_CTX_new(SSLv23_client_method());
	//printf("ssl3\n");
	if(! SSL_CTX_load_verify_locations(ctx, "/home/.cert/TrustStore.pem", NULL)){
		fprintf(stderr, "Error loading trust store\n");
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return 0;
		}

	//printf("ssl11\n");
	 /* Setup the connection */
	bio = BIO_new_ssl_connect(ctx);

	/* Set the SSL_MODE_AUTO_RETRY flag */
	BIO_get_ssl(bio, & ssl);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	//printf("ssl12\n");
	/* Create and setup the connection */
	BIO_set_conn_hostname(bio, "lronetto.com:https");

	//printf("ssl13\n");
	/* Verify the connection opened and perform the handshake */

	if(BIO_do_connect(bio) <= 0){
		fprintf(stderr, "Error attempting to connect\n");
		ERR_print_errors_fp(stderr);
		BIO_free_all(bio);
		SSL_CTX_free(ctx);
		return 0;
		}
	if(SSL_get_verify_result(ssl) != X509_V_OK){
		fprintf(stderr, "Certificate verification error: %i\n", SSL_get_verify_result(ssl));
		BIO_free_all(bio);
		SSL_CTX_free(ctx);
		return 0;
		}

	memset(obuf,0,sizeof(obuf));
	sprintf(obuf,   "POST /ssa/scripts/gate_xbee.php  HTTP/1.0\r\n"
					"Host: lronetto.com\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Content-Length: %d \r\n"
					"Connection: Close\r\n\r\n"
					"%s\r\n",
					strlen(post),post);

	BIO_write(bio, obuf, strlen(obuf));

	for(;;)
	    {
	        tam = BIO_read(bio, r, 1023);
	        if(tam <= 0) break;
	        r[tam] = 0;
	        printf("%s", r);
	    }
	BIO_free_all(bio);
	SSL_CTX_free(ctx);
	//printf("teste\n");
	return 1;

	}

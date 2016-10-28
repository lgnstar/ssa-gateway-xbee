#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/types.h>
#include "serial.h"

#define BAUDRATE B9600

char devicename[80] = "/dev/ttyO4", ch;
int fd;
int serial_init(void *func){
	struct termios newtio;
		struct sigaction saio;

		//
		//open the device in non-blocking way (read will return immediately)
		fd = open(devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (fd < 0){
			perror(devicename);
			exit(1);
			}
		//
		//install the serial handler before making the device asynchronous
		saio.sa_handler = func;
		sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
		saio.sa_flags = 0;
		saio.sa_restorer = NULL;
		sigaction(SIGIO,&saio,NULL);
		//
		// allow the process to receive SIGIO
		fcntl(fd, F_SETOWN, getpid());
		//
		// make the file descriptor asynchronous
		fcntl(fd, F_SETFL, FASYNC);
		//
		cfmakeraw(&newtio);
		// set new port settings for canonical input processing
		newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
		newtio.c_iflag = IGNPAR;
		newtio.c_oflag = 0;
		newtio.c_lflag = 0;
		newtio.c_cc[VMIN]=1;
		newtio.c_cc[VTIME]=0;
		tcflush(fd, TCIFLUSH);
		tcsetattr(fd,TCSANOW,&newtio);
		//cfsetospeed(&newtio,BAUDRATE);            // 115200 baud
		//cfsetispeed(&newtio,BAUDRATE);
		return fd;
}

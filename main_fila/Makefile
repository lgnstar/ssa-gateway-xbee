TARGET = ssa_main
LIBS = -lm -lssl -lcrypto -lpthread
CC = gcc
CFLAGS = -g

.PHONY: default all clean

default: $(TARGET)
all: default

INC=-I../include -I./

SRC = ../lib/serial.c ../lib/xbee.c ../lib/ssl.c ../lib/socket.c $(wildcard *.c)
OBJ= $(SRC:.c=.o)

%.o: %.c  
	$(CC) $(CFLAGS) $(INC) -c $< -o $@ 

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LIBS) $(INC) -o $@

teste:
	echo $(SRC)
	echo $(OBJ)

cpp:
	cppcheck /home/ssa/*
	
clean:
	-rm -f *.o
	-rm -f ../lib/*.o
	-rm -f $(TARGET)

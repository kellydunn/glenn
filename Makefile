LDFLAGS=$(shell pkg-config --libs libusb-1.0)
CFLAGS = $(shell pkg-config --cflags libusb-1.0)
CC=gcc 

all:
	$(CC) src/glenn.c $(CFLAGS) $(LDFLAGS) -o bin/glenn

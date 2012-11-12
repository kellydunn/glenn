LDFLAGS=$(shell pkg-config --libs libusb-1.0 liblo)
CFLAGS = $(shell pkg-config --cflags libusb-1.0 liblo)
CC=gcc 

all:
	$(CC) src/glenn.c $(CFLAGS) $(LDFLAGS) -o bin/glenn
clean:
	rm bin/*
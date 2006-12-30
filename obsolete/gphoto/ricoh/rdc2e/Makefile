# Makefile for rdc2e

CC = gcc
CFLAGS = -O2 -Wall
LDFLAGS = -s
LIBS =

all: rdc2e

rdc2e: rdc2e.o
	$(CC) $(CFLAGS) -o rdc2e rdc2e.o

rdc2e.o: rdc2e.c
	$(CC) $(CFLAGS) -c rdc2e.c

install: rdc2e
	install -c -o bin -g bin -m 555 rdc2e /usr/local/bin/
	install -c -o bin -g bin -m 555 j6iextract /usr/local/bin/
	install -c -o bin -g bin -m 444 rdc2e.man /usr/local/man/man1/rdc2e.1
	install -c -o bin -g bin -m 444 j6iextract.man /usr/local/man/man1/j6iextract.1

clean:
	rm -f core *.o rdc2e *~

uninstall:
	rm /usr/local/bin/rdc2e
	rm /usr/local/man/man1/rdc2e.1
	rm /usr/local/bin/j6iextract
	rm /usr/local/man/man1/j6iextract.1

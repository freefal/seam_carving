CC=gcc
LIBS=-lm
CFLAGS=-Wall -O3
PREFIX = /usr/local

seam_carver : seam_carver.o
	$(CC) -o seam_carver seam_carver.o $(CFLAGS) $(LIBS)

seam_carver.o : seam_carver.c
	$(CC) -c seam_carver.c $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f seam_carver seam_carver.o

.PHONY: install
install: seam_carver
	mkdir -p $(PREFIX)/bin
	cp $< $(PREFIX)/bin/seam_carver

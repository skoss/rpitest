VERSION = 3.02
CC      = gcc
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
LDFLAGS = -lm -lpthread `gtk-config --cflags` `gtk-config --libs` -lgthread

OBJ = ks0108.o

prog: $(OBJ)
	$(CC) $(CFLAGS) -o prog $(OBJ) $(LDFLAGS) -I.

%.o: %.c
	$(CC) $(CFLAGS) -c $< -I.
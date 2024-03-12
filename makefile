CC=gcc
CFLAGS=-lm

index:  index.c
	$(CC) index.c -o index $(CFLAGS)

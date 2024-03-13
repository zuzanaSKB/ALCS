CC=gcc
CFLAGS=-lm

make all: grammar index

grammar: grammar.c grammar.h
	$(CC) grammar.c -c -o grammar.o $(CFLAGS)

index:  index.c grammar.h
	$(CC) index.c grammar.o -o index $(CFLAGS)

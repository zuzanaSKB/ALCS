CC=gcc
CFLAGS=-lm

all: grammar index

grammar: grammar.c grammar.h
	$(CC) grammar.c -c -o grammar $(CFLAGS)

index: index.c grammar.h grammar.c
	$(CC) index.c grammar.c -o index $(CFLAGS)

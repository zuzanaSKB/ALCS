CC=gcc
CFLAGS=-lm

all: grammar index queries

grammar: grammar.c grammar.h
	$(CC) grammar.c -c -o grammar $(CFLAGS)

index: index.c grammar.h grammar.c
	$(CC) index.c grammar.c -o index $(CFLAGS)
	
queries: queries.c grammar.h grammar.c
	$(CC) queries.c grammar.c -o queries $(CFLAGS)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

typedef struct {
    unsigned int left, right; 
} Tpair;

extern Tpair *R; // rules
extern unsigned int sizeRules;
extern unsigned long long *hashN; //hashes for all nonterminals
extern unsigned int *sizeN; //size of each nonterminal

extern const unsigned long long p; // fixed prime number
extern const unsigned long long c; // randomly chosen positive integer
extern const unsigned long long cInv; //inverse number to c according to mod p

unsigned long long fingerprint(unsigned long long terminal);
unsigned long long power(unsigned long long a, unsigned int k);
unsigned int getSize(unsigned int X);
unsigned long long getHash(unsigned int X);
void sizeNonTerminal();
void hashNonterminal();
unsigned long long concate(unsigned int left, unsigned int right);
unsigned long long hashSubstringToI(unsigned int i);
unsigned long long *prefixB(float e, unsigned int X);
unsigned long long hashSubstring(unsigned int i, unsigned int j);
void readInput(int argc, char **argv);

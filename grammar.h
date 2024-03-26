#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
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
extern uint64_t *hashN; //hashes for all nonterminals
extern unsigned int *sizeN; //size of each nonterminal
extern unsigned int *indicesOfExpX; //indices of first occurence of exp(X)

extern const uint64_t c; // randomly chosen positive integer
extern const uint64_t cInv; //inverse number to c according to mod p
extern const uint64_t p; // fixed prime number
extern const unsigned int offset;


uint64_t fingerprint(uint64_t terminal);
uint64_t power(uint64_t a, unsigned int k);
unsigned int getSize(unsigned int X);
uint64_t getHash(unsigned int X);
void sizeNonTerminal();
void hashNonterminal();
uint64_t concate(unsigned int left, unsigned int right);
uint64_t recurrentPref (unsigned int i, unsigned int X);
uint64_t hashSubstringToI(unsigned int i);
uint64_t hashSubstring(unsigned int i, unsigned int j);
uint64_t *prefixB(float e, unsigned int X);
void computeIndicesOfExpX(unsigned int X, unsigned int pos);
uint64_t** hashBlock(float e);
void readInput(int argc, char **argv);

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

typedef struct {
    uint64_t key;
    unsigned int value; 
} THashPair;

extern char output[1024];
extern Tpair *R; // rules
extern THashPair *isPrefBlock;
extern THashPair *isSufBlock;
extern unsigned int sizeRules;
extern uint64_t *hashN; //hashes for all nonterminals
extern unsigned int *sizeN; //size of each nonterminal
extern unsigned int *indicesOfExpX; //indices of first occurence of exp(X)

extern float e; //from input
extern const uint64_t c; // randomly chosen positive integer generated from random.org
extern const uint64_t cInv; // inverse number to c in mod p, computed in inverseNum.c
extern const uint64_t p; // fixed prime number
extern const unsigned int offset;

uint64_t mul_mod_mersenne(const uint64_t a, const uint64_t b, const uint64_t k);
uint64_t mod_mersenne(uint64_t a, const uint64_t k);
uint64_t  pow_mod_mersenne(const uint64_t a, uint64_t n, const uint64_t k);
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
void hashBlock(float e);
void readInput(int argc, char **argv);

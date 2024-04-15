#include "grammar.h"

Tpair *R;
THashPair *isPrefBlock;
THashPair *isSufBlock;
unsigned int sizeRules;
uint64_t *hashN;
unsigned int *sizeN;
unsigned int *indicesOfExpX;

float e;
const uint64_t c = 28222; //random number generated from random.org
const uint64_t cInv = 1738410520411018574; //inverse number to c in mod p, computed in inverseNum.c
const uint64_t p = (1ULL << 61) - 1;
const unsigned int offset = 256;

//=============================================================================
//function mul_mod_mersenne is from https://github.com/dominikkempa/lz77-to-slp/blob/main/src/karp_rabin_hashing.cpp#L55
uint64_t mul_mod_mersenne(
    const uint64_t a,
    const uint64_t b,
    const uint64_t k) {
  const uint64_t p = ((uint64_t)1 << k) - 1;
  __extension__ const unsigned __int128 ab =
    (unsigned __int128)a *
    (unsigned __int128)b;
  uint64_t lo = (uint64_t)ab;
  const uint64_t hi = (ab >> 64);
  lo = (lo & p) + ((lo >> k) + (hi << (64 - k)));
  lo = (lo & p) + (lo >> k);
  return lo == p ? 0 : lo;
}

//=============================================================================

// power computes c^k
uint64_t power(uint64_t a, unsigned int k) {
    uint64_t result = 1;
    for (unsigned int i = 0; i < k; i++) {
        result = mul_mod_mersenne(result, a, 61);
    }
    return result;
}

// hash terminals
uint64_t fingerprint(uint64_t terminal) {
    return terminal * c % p;
}

unsigned int getSize(unsigned int X) {
    if (X < offset) {
        return 1;
    } else {
        return sizeN[X-offset];
    }
}

uint64_t getHash (unsigned int X) {
    if (X < offset) {
        return fingerprint(X);
    } else {
        return getHash(R[X-offset].left) + power(c, getSize(R[X-offset].left)) * getHash(R[X-offset].right) % p;
    }
}

void sizeNonTerminal () { //computes size of all nonterminals and store it to sizeN
    sizeN = (void *)malloc(sizeRules * sizeof(unsigned int));
    for (unsigned int i = 0; i < sizeRules; i++) { 
        sizeN[i] = 0;
        sizeN[i] += getSize(R[i].left);
        sizeN[i] += getSize(R[i].right);

        //test purpose
        //printf("%u %u\n", i, sizeN[i]);
    }
}

void hashNonterminal() { //computes hashes for all nonterminals
    hashN = (void *)malloc(sizeRules * sizeof(uint64_t));
    for (unsigned int i = 0; i < sizeRules; i++) {
        hashN[i] = getHash(i+offset);
    }
}

uint64_t concate(unsigned int left, unsigned int right) { // computes hashes for 2 nonterminals or terminals
    //printf("left, right: %" PRIu64 " %" PRIu64 "\n", hashN[left - offset], hashN[right - offset]);
    //printf("power: %" PRIu64 "\n", power(c, sizeN[left - offset]));    

    return getHash(left) + power(c, getSize(left)) * getHash(right) % p;

}

//returns first i hashes of X <=> computes S[1...n]
uint64_t recurrentPref (unsigned int i, unsigned int X) {
    if (getSize(X) == i) {
        return getHash(X); 
    }
    //test purposes
    /* printf("reccurentPref i: %u X: %u\n", i, X);
    printf("reccurentPref size X.left: %u\n", getSize(R[X-offset].left));
    */
    if (getSize(R[X-offset].left) == i) {
        //exact left subtree
        return getHash(R[X-offset].left);
    } 
    if (getSize(R[X-offset].left) < i) {
        //all left subtree + something from right subtree
        //return concate(R[X-offset].left, recurrentPref( i- getSize(R[X-offset].left),R[X-offset].right));

        //i know this is awful, ... but it works and my brain doesnt work anymore:
        return getHash(R[X-offset].left) + power(c, getSize(R[X-offset].left)) * 
                recurrentPref( i- getSize(R[X-offset].left),R[X-offset].right) % p;
    } 
    if (getSize(R[X-offset].left) > i) {
        //deeper to the left subtree
        return recurrentPref( i, R[X-offset].left);
    } 
}

uint64_t hashSubstringToI(unsigned int i) {
    //printf ("hashSubstring root: %u\n", sizeRules+offset-1);
    return recurrentPref(i, sizeRules + offset -1);
}

//computes hash of S[i..j]
uint64_t hashSubstring(unsigned int i, unsigned int j) {
    if (i < 1 || j < 1 || i > sizeN[sizeRules-1] || j > sizeN[sizeRules-1]) {
        printf("Error! Cannot compute hash of substring, wrong arguments.\n");
        return 0;
    }
    if (i == 1) {
        return hashSubstringToI(j);
    }
    uint64_t hashI = hashSubstringToI(i-1); //hash of S[1..i]
    uint64_t hashJ = hashSubstringToI(j); //hash of S[1..j]
    
    return mul_mod_mersenne(hashJ - hashI, power(cInv, i-1), 61);
}

//returns array of indices of starts of exp(X), where X is terminal or nonterminal
void computeIndicesOfExpX(unsigned int X, unsigned int pos) {
    if (X < offset) {
        if(indicesOfExpX[X] == 0){
            indicesOfExpX[X] = pos;
        }
    } else {
        if(indicesOfExpX[X] == 0){
            indicesOfExpX[X] = pos;
        }
        computeIndicesOfExpX(R[X-offset].left, pos);
        computeIndicesOfExpX(R[X-offset].right, pos + getSize(R[X-offset].left));
    }
}

void readInput(int argc, char **argv) {
    FILE *Pf;
    char fname[1024];
    fputs("==== Command line:\n", stderr);
    for (int i = 0; i < argc; i++)
    fprintf(stderr, " %s", argv[i]);
    fputs("\n", stderr);
    if (argc != 3) {
        fprintf(stderr,
                "Usage: %s <filename> <e>\n"
                "This script constructs index from <filename>.plainslp\n"
                "where e is float from (0,1)\n"
                "by supporting Karp-Rabin fingerprint queries. \n",
                argv[0]);
        exit(1);
    }


    // read .plainslp file
    strcpy(fname, argv[1]);
    strcat(fname, ".plainslp"); 
    Pf = fopen(fname, "r");
    if (Pf == NULL) {
        fprintf(stderr, "Error: cannot open file %s for reading\n", fname);
        exit(1);
    }
    unsigned int a, b;
    sizeRules = 0;
    while (fscanf(Pf, "%u %u\n", &a, &b) == 2) {
        sizeRules ++;
    }
    // rewind file pointer
    rewind(Pf);
    // allocate array of rules
    R = (void *)malloc(sizeRules * sizeof(Tpair));
    if (R == NULL) {
        //printf(stderr, "Error: memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < sizeRules; i++) {
        fscanf(Pf, "%u %u\n", &R[i].left, &R[i].right);
    }

    //read e
    e = atof(argv[2]);
    if (e <= 0 || e >= 1) {
        printf("Error! Float e must be between 0 and 1.\n");
        exit(1);
    }
    printf("loaded e : %f\n", e);

    /* sizeNonTerminal();
    hashNonterminal();

    //initialization & computing of array with first occurencies of exp(X) of all terminals and nonterminals
    indicesOfExpX = (void *)calloc((sizeRules+offset), sizeof(unsigned int));
    computeIndicesOfExpX(sizeRules + offset-1, 1);


    hashBlock(e); */
    

    ////////// some unit tests //////////
    //test1: read from .plaintslp
    /* int j = 0;
    while (j<sizeRules) {
        printf("%u %u\n", R[j].left, R[j].right);
        j++;
    } */

    //test1.5: sizeNonTerminal
    /* printf ("size of nonterminals: \n");
    for (unsigned int i = 1; i <= sizeRules; i++) {
        printf ("%u %u\n", i, sizeN[i-1]);
    } */

    //test2: comparing concate & hashN
    //print prime, c
    /* printf("prime: %" PRIu64 "\n", p);
    printf("c: %" PRIu64 "\n", c);
    uint64_t h = concate(257,258);
    //print result of concate
    printf("concate: %" PRIu64 "\n", h);
    //print result of hashN
    printf("hashN: %" PRIu64 "\n", hashN[5]);
    if (h == hashN[5]) {
        printf("test2 succeeded\n");
    } else {
        printf("E stupido!!!\n");
    }
 */
    //test4: getHash
    /* uint64_t h2 = getHash(261);
    if (h2 == hashN[5]) {
        printf("test4 succeeded\n");
    } else {
        printf("E stupido!!!\n");
    } */
    
    //test3: print all hashes of nontermonals
    /* for (unsigned int i = 1; i <= sizeRules; i++) {
        printf ("%u %" PRIu64 "\n", i, hashN[i-1]);
    } */

    //test5: prefixB
    //uint64_t *hashP = prefixB(e, 261);

    //test6.: reccurentPref
    /* uint64_t r = recurrentPref(2, 261);
    printf("recurrentPref r: %" PRIu64 "\n", r);
    */
    //test7: hashSubstringToI
    /* unsigned int i = 3;
    uint64_t hSub = hashSubstringToI(i);
    printf("hashSubstringToI i: %u hash: %" PRIu64 "\n", i, hSub); */

    //test7.5: cInv
    /* uint64_t result = mul_mod_mersenne(c, cInv, 61);
    printf("test7.5: %" PRIu64 " * %" PRIu64 " mod %" PRIu64 " should equal 1: %" PRIu64 "\n", c, cInv, p, result);
 */
    //test8: hashSubstring S[i..j]
    /* uint64_t hS = hashSubstring(2, 3);
    printf("test8 hash: %" PRIu64 "\n", hS);
    if (hS == getHash(offset)){
        printf("test8: succeeded!\n");
    } else {
        printf("test8: retardo e stupido!\n");
    } */

    //test9: computeIndicesOfExpX
    /* printf("test9: computeIndicesOfExpX\n");
    for(unsigned int i = 0; i < sizeRules+offset; i++) {
        if(indicesOfExpX[i] != 0) {
            printf("start of exp(%u) : %u\n", i, indicesOfExpX[i]);
        }
    } */

    //free(hashP);
    fclose(Pf);

}


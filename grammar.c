#include "grammar.h"

char output[1024];
Tpair *R;
THashPair *isPrefBlock;
THashPair *isSufBlock;
unsigned int sizeRules;
uint64_t *hashN;
unsigned int *sizeN;
unsigned int *indicesOfExpX;

float e;
const uint64_t c = 28222;
const uint64_t cInv = 1738410520411018574;
const uint64_t p = (1ULL << 61) - 1;
const unsigned int offset = 256;

//=============================================================================
//functions mul_mod_mersenne, pow_mod_mersenne and mod_mersenne are from 
//https://github.com/dominikkempa/lz77-to-slp/blob/main/src/karp_rabin_hashing.cpp#L55

//=============================================================================
// Return (a * b) mod p, where p = (2^k) - 1.
// Requires a, b <= 2^k. Tested for k = 1, .., 63.
//=============================================================================
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
uint64_t mod_mersenne(
    uint64_t a,
    const uint64_t k) {
  uint64_t p = ((uint64_t)1 << k) - 1;
  if (k < 32) {

    // We need to check if a <= 2^(2k).
    const uint64_t threshold = ((uint64_t)1 << (k << 1));
    if (a <= threshold) {
      a = (a & p) + (a >> k);
      a = (a & p) + (a >> k);
      return a == p ? 0 : a;
    } else return a % p;
  } else {

    // We are guaranteed that a < 2^(2k)
    // because a < 2^64 <= 2^(2k).
    a = (a & p) + (a >> k);
    a = (a & p) + (a >> k);
    return a == p ? 0 : a;
  }
}
//=============================================================================
// Return (a^n) mod p, where p = (2^k) - 1.
//=============================================================================
uint64_t  pow_mod_mersenne(
    const uint64_t a,
    uint64_t n,
    const uint64_t k) {
  uint64_t pow = mod_mersenne(a, k);
  uint64_t ret = mod_mersenne(1, k);
  while (n > 0) {
    if (n & 1)
      ret = mul_mod_mersenne(ret, pow, k);
    pow = mul_mod_mersenne(pow, pow, k);
    n >>= 1;
  }
  return ret;
}
//=============================================================================

// power computes a^k
uint64_t power(uint64_t a, unsigned int k) {
    return pow_mod_mersenne(a, k, 61);
}

// hash terminals
uint64_t fingerprint(uint64_t terminal) {
    return (terminal * c) % p;
}

// computes size of all nonterminals and store it to sizeN
void sizeNonTerminal () {
    sizeN = (void *)malloc(sizeRules * sizeof(unsigned int));
    for (unsigned int i = 0; i < sizeRules; i++) { 
        sizeN[i] = 0;
        sizeN[i] += getSize(R[i].left);
        sizeN[i] += getSize(R[i].right);
    }
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
        return hashN[X-offset];
    }
}

uint64_t reccurentHash(unsigned int X) {
    if (X < offset) {
        return fingerprint(X);
    } else {
        uint64_t right = mul_mod_mersenne(power(c, getSize(R[X-offset].left)), getHash(R[X-offset].right), 61) %p;
        return (getHash(R[X-offset].left) + right) % p;        
    }
}

// computes hashes for all nonterminals
void hashNonterminal() {
    hashN = (void *)malloc(sizeRules * sizeof(uint64_t));
    for (unsigned int i = 0; i < sizeRules; i++) {
        hashN[i] = reccurentHash(i+offset);
        //test purpose
        printf("hashed %u of %u\n", i+1, sizeRules);
    }
}

// computes hashes for 2 nonterminals or terminals
uint64_t concate(unsigned int left, unsigned int right) {

    return (getHash(left) + mul_mod_mersenne(power(c, getSize(left)), getHash(right), 61)) % p;
}

// returns first i hashes of X <=> computes S[1...n]
uint64_t recurrentPref (unsigned int i, unsigned int X) {
    if (getSize(X) == i) {
        return getHash(X); 
    }
    if (getSize(R[X-offset].left) == i) {
        //exact left subtree
        return getHash(R[X-offset].left);
    } 
    if (getSize(R[X-offset].left) < i) {
        //all left subtree + something from right subtree

        return (getHash(R[X-offset].left) + mul_mod_mersenne(power(c, getSize(R[X-offset].left)),
                recurrentPref( i- getSize(R[X-offset].left),R[X-offset].right), 61)) % p;
    } 
    if (getSize(R[X-offset].left) > i) {
        //deeper to the left subtree
        return recurrentPref( i, R[X-offset].left);
    } 
}

// computes hash of S[1..i]
uint64_t hashSubstringToI(unsigned int i) {
    return recurrentPref(i, sizeRules + offset -1);
}

// computes hash of S[i..j]
uint64_t hashSubstring(unsigned int i, unsigned int j) {
    if (i < 1 || j < 1 || i > sizeN[sizeRules-1] || j > sizeN[sizeRules-1] || i > j)     {
        printf("Error! Cannot compute hash of substring, wrong arguments.\n");
        return 0;
    }
    if (i == 1) {
        return hashSubstringToI(j);
    }
    uint64_t hashI = hashSubstringToI(i-1); // hash of S[1..i]
    uint64_t hashJ = hashSubstringToI(j); // hash of S[1..j]
    
    return mul_mod_mersenne(hashJ - hashI, power(cInv, i-1), 61);
}

// computes an array of indices of starts of exp(X), where X is terminal or nonterminal
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
                "This script constructs an index from <filename>.plainslp\n"
                "where e is the float from (0,1)\n"
                "by supporting Karp-Rabin fingerprint queries. \n",
                argv[0]);
        exit(1);
    }


    // read .plainslp file
    strcpy(fname, argv[1]);
    strcat(fname, ".plainslp");
    strcpy(output, argv[1]);
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
    //test3: print all hashes of nontermonals
    /* for (unsigned int i = 1; i <= sizeRules; i++) {
        printf ("%u %" PRIu64 "\n", i, hashN[i-1]);
    } */

    //test4: getHash
    /* uint64_t h2 = getHash(261);
    if (h2 == hashN[5]) {
        printf("test4 succeeded\n");
    } else {
        printf("E stupido!!!\n");
        printf("getHash: %" PRIu64 "  hashN[5]: %" PRIu64 "\n", h2, hashN[5]);
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

    //for test5
    //free(hashP);
    fclose(Pf);

}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <time.h>


typedef struct {
    unsigned int left, right;
} Tpair;

Tpair *R; // rules
unsigned int sizeRules;
unsigned long long *hashN; //hashes for all nonterminals
unsigned int *sizeN; //size of each nonterminal

unsigned long long p = (1ULL << 61) - 1; // fixed prime number
unsigned int c ; // randomly chosen positive integer

void genRanC () {
    srand(time(NULL));
    c = rand() % (1000) + 256; //generate a random number greater than 256
}

void sizeNonTerminal () { //computes size of all nonterminals and store it to sizeN
    sizeN = (void *)malloc(sizeRules * sizeof(unsigned int));
    for (unsigned int i = 0; i < sizeRules; i++) {
        if (R[i].left < 256 ) {
            sizeN[i]++;
        }
        if (R[i].right < 256 ) {
            sizeN[i]++;
        }
        if (R[i].left >= 256 ) {
            sizeN[i]+= sizeN[R[i].left - 256];
        }
        if (R[i].right >= 256 ) {
            sizeN[i]+= sizeN[R[i].right - 256];
        }
        //test purpose
        //printf("%u %u\n", i, sizeN[i]);
    }
}

// hash terminals
unsigned long long fingerprint(unsigned long long terminal) {
    return terminal * c % p;
}
// powerC computes c^k
unsigned long long powerC (unsigned int k) {
    unsigned long long result = 1;
    for (unsigned int i = 0; i < k; i++) {
        result = result * c;
    }
    return result;
}

void hashNonterminal() { //computes hashes for all nonterminals
    hashN = (void *)malloc(sizeRules * sizeof(unsigned long long));
    for (unsigned int i = 0; i < sizeRules; i++) {
        // 2 teminals
        if (R[i].left < 256 && R[i].right < 256 ) {
            hashN[i] = fingerprint(R[i].left) + c * fingerprint(R[i].right) % p;
        }
        // terminal and nonterminal
        if (R[i].left < 256 && R[i].right >= 256 ) {
            hashN[i] = fingerprint(R[i].left) + c * hashN[R[i].right - 256] % p;
        }
        // nonterminal and terminal
        if (R[i].left >= 256 && R[i].right < 256 ) {
            hashN[i] = hashN[R[i].left - 256] + powerC(sizeN[R[i].left - 256]) * fingerprint(R[i].right) % p;
        }
        // nonterminal and nonterminal
        if (R[i].left >= 256 && R[i].right >= 256 ) {
            hashN[i] = hashN[R[i].left - 256] + powerC(sizeN[R[i].left - 256]) * hashN[R[i].right - 256] % p;
        }
    }
}

unsigned long long concate(unsigned int left, unsigned int right) { // computes hashes for 2 nonterminals or terminals
    //printf("left, right: %llu %llu\n", hashN[left - 256], hashN[right - 256]);
    //printf("powerC: %llu\n", powerC(sizeN[left - 256]));    

    // 2 teminals
    if (left < 256 && right < 256 ) {
        return fingerprint(left) + c * fingerprint(right) % p;
    }
    // terminal and nonterminal
    if (left < 256 && right >= 256 ) {
        return fingerprint(left) + c * hashN[right - 256] % p;
    }
    // nonterminal and terminal
    if (left >= 256 && right < 256 ) {
        return hashN[left - 256] + powerC(sizeN[left - 256]) * fingerprint(right) % p;
    }
    // nonterminal and nonterminal
    if (left >= 256 && right >= 256 ) {
        return hashN[left - 256] + powerC(sizeN[left - 256]) * hashN[right - 256] % p;
    }
}

unsigned long long recurrentPref (unsigned int i, unsigned int X) {
    if (X < 256 && i == 1) {
        //X is terminal
        return fingerprint(X);
    }
    if (R[X-256].left < 256 && i == 1) {
        //left is terminal
        return fingerprint(R[X-256].left);
    }
    if (sizeN[R[X-256].left] == i) {
        return hashN[R[X-256].left];
    } 
    if (sizeN[R[X-256].left] < i) {
        return concate(R[X-256].left, recurrentPref( i- sizeN[R[X-256].left],R[X-256].right));
    } 
    if (sizeN[R[X-256].left] > i) {
        return recurrentPref( i, R[X-256].left);
    } 
}

void prefixB(float e, unsigned int X) { //given e = <0,1>, nonterminal X
    //compute k
    unsigned int maxPref = 0;
    for (unsigned int k = 0; pow((1/(1-e)), k) < sizeN[X-256]; k++) {
        maxPref = (int) pow((1/(1-e)), k);
    }
    for (unsigned int i = 1; i <= maxPref; i++) {
        recurrentPref(i, X); // where to save ?
    }
    
}

int main(int argc, char **argv) {
    FILE *Pf;
    char fname[1024];
    char outname[1024];
    fputs("==== Command line:\n", stderr);
    for (int i = 0; i < argc; i++)
    fprintf(stderr, " %s", argv[i]);
    fputs("\n", stderr);
    if (argc != 3) {
        fprintf(stderr,
                "Usage: %s <filename> <e>\n"
                "This scipt constructs index from <filename>.plainslp\n"
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
    float e = atof(argv[2]);
    if (e <= 0 || e >= 1) {
        printf("Error! Float e must be between 0 and 1.\n");
        exit(1);
    }
    printf("loaded e : %f\n", e);

    sizeNonTerminal();
    genRanC();
    hashNonterminal();

    ////////// some unit tests //////////
    //test1: read from .plaintslp
    /* int j = 0;
    while (j<sizeRules) {
        printf("%u %u\n", R[j].left, R[j].right);
        j++;
    } */

    //test2: comparing concate & hashN
    //print prime, c
    printf("prime: %llu\n", p);
    printf("c: %u\n", c);
    unsigned long long h = concate(257,258);
    //print result of concate
    printf("concate: %llu\n", h);
    //print result of hashN
    printf("hashN: %llu\n", hashN[5]);
    if (h == hashN[5]) {
        printf("test succeeded\n");
    } else {
        printf("E stupido!!!\n");
    }
    
    //test3: print all hashes of nontermonals
    /* for (unsigned int i = 1; i <= sizeRules; i++) {
        printf ("%u %llu\n", i, hashN[i-1]);
    } */
    
    //free memory
    free(R);
    free(sizeN);
    free(hashN);
    fclose(Pf);
    return 0;
}


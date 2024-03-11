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

Tpair *R; // rules
unsigned int sizeRules;
unsigned long long *hashN; //hashes for all nonterminals
unsigned int *sizeN; //size of each nonterminal

unsigned long long p = (1ULL << 61) - 1; // fixed prime number
unsigned int c ; // randomly chosen positive integer

void genRanC () {
    srand(time(NULL));
    c = rand() % (1000) + 256; //generate a random number greater than 256
    c = 780; //for test purposes
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

unsigned int getSize(unsigned int X) {
    if (X < 256) {
        return 1;
    } else {
        return sizeN[X-256];
    }
}

unsigned long long getHash (unsigned int X) {
    if (X < 256) {
        return fingerprint(X);
    } else {
        return getHash(R[X-256].left) + powerC(getSize(R[X-256].left)) * getHash(R[X-256].right) % p;
    }
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

void hashNonterminal() { //computes hashes for all nonterminals
    hashN = (void *)malloc(sizeRules * sizeof(unsigned long long));
    for (unsigned int i = 0; i < sizeRules; i++) {
        hashN[i] = getHash(i+256);
    }
}

unsigned long long concate(unsigned int left, unsigned int right) { // computes hashes for 2 nonterminals or terminals
    //printf("left, right: %llu %llu\n", hashN[left - 256], hashN[right - 256]);
    //printf("powerC: %llu\n", powerC(sizeN[left - 256]));    

    return getHash(left) + powerC(getSize(left)) * getHash(right) % p;

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

/* unsigned long long * prefixB(float e, unsigned int X) { //given e = <0,1>, nonterminal X
    unsigned long long *hashP; //hashes for prefix block

    //compute k
    unsigned int maxPref = 0;
    for (unsigned int k = 0; pow((1/(1-e)), k) < sizeN[X-256]; k++) {
        maxPref = (int) pow((1/(1-e)), k);
    }
    hashP = (void *)malloc(maxPref * sizeof(unsigned long long));
    for (unsigned int i = 1; i <= maxPref; i++) {
        hashP[i-1] = recurrentPref(i, X); //save into hashtable later
        //test
        printf("prefix of length: %u hash: %llu\n", i, hashP[i-1]);
    }
    return hashP;
} */

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
        printf("test2 succeeded\n");
    } else {
        printf("E stupido!!!\n");
    }

    //test4: getHash
    unsigned long long h2 = getHash(261);
    if (h2 == hashN[5]) {
        printf("test4 succeeded\n");
    } else {
        printf("E stupido!!!\n");
    }
    
    //test3: print all hashes of nontermonals
    /* for (unsigned int i = 1; i <= sizeRules; i++) {
        printf ("%u %llu\n", i, hashN[i-1]);
    } */

    //test5: prefixB
    //unsigned long long *hashP = prefixB(e, 261);

    
    //free memory
    free(R);
    free(sizeN);
    free(hashN);
    //free(hashP);
    fclose(Pf);
    return 0;
}


#include "grammar.h"

Tpair *R;
unsigned int sizeRules;
unsigned long long *hashN;
unsigned int *sizeN;

const unsigned long long c = 28222; //generated from random.org
const unsigned long long cInv = 1738410520411018574;
const unsigned long long p = (1ULL << 61) - 1;

// power computes c^k
unsigned long long power (unsigned long long a, unsigned int k) {
    unsigned long long result = 1;
    for (unsigned int i = 0; i < k; i++) {
        result = result * a;
    }
    return result;
}


// hash terminals
unsigned long long fingerprint(unsigned long long terminal) {
    return terminal * c % p;
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
        return getHash(R[X-256].left) + power(c, getSize(R[X-256].left)) * getHash(R[X-256].right) % p;
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
    hashN = (void *)malloc(sizeRules * sizeof(unsigned long long));
    for (unsigned int i = 0; i < sizeRules; i++) {
        hashN[i] = getHash(i+256);
    }
}

unsigned long long concate(unsigned int left, unsigned int right) { // computes hashes for 2 nonterminals or terminals
    //printf("left, right: %llu %llu\n", hashN[left - 256], hashN[right - 256]);
    //printf("power: %llu\n", power(c, sizeN[left - 256]));    

    return getHash(left) + power(c, getSize(left)) * getHash(right) % p;

}

//returns first i hashes of X <=> computes S[1...n]
unsigned long long recurrentPref (unsigned int i, unsigned int X) {
    if (getSize(X) == i) {
        return getHash(X); 
    }
    //test purposes
    printf("reccurentPref i: %u X: %u\n", i, X);
    printf("reccurentPref size X.left: %u\n", getSize(R[X-256].left));

    if (getSize(R[X-256].left) == i) {
        //exact left subtree
        return getHash(R[X-256].left);
    } 
    if (getSize(R[X-256].left) < i) {
        //all left subtree + something from right subtree
        //return concate(R[X-256].left, recurrentPref( i- getSize(R[X-256].left),R[X-256].right));

        //i know this is awful, ... but it works and my brain doesnt work anymore:
        return getHash(R[X-256].left) + power(c, getSize(R[X-256].left)) * 
                recurrentPref( i- getSize(R[X-256].left),R[X-256].right) % p;
    } 
    if (getSize(R[X-256].left) > i) {
        //deeper to the left subtree
        return recurrentPref( i, R[X-256].left);
    } 
}

unsigned long long hashSubstringToI(unsigned int i) {
    printf ("hashSubstring root: %u\n", sizeRules+256-1);
    return recurrentPref(i, sizeRules + 256 -1);
}

//computes all prefix blocks of nonterminal X
unsigned long long * prefixB(float e, unsigned int X) { //given e = <0,1>, nonterminal X
    unsigned long long *hashP; //hashes for prefix block

    //compute k
    unsigned int maxPref = 0;
    for (unsigned int k = 0; pow((1/(1-e)), k) < sizeN[X-256]; k++) {
        maxPref = (int) pow((1/(1-e)), k);
    }
    hashP = (void *)malloc(maxPref * sizeof(unsigned long long));
    for (unsigned int i = 1; i <= maxPref; i++) {
        hashP[i-1] = recurrentPref(i, X); //save into hashtable later
        //test purpose
        printf("prefix of length: %u hash: %llu\n", i, hashP[i-1]);
    }
    return hashP;
}

//computes hash of S[i..j]
unsigned long long hashSubstring(unsigned int i, unsigned int j) {
    if (i < 1 || j < 1 || i > sizeN[sizeRules-1] || j > sizeN[sizeRules-1]) {
        printf("Error! Cannot compute hash of substring, wrong arguments.\n");
        return 0;
    }
    if (i == 1) {
        return hashSubstringToI(j);
    }
    unsigned long long hashI = hashSubstringToI(i-1); //hash of S[1..i]
    unsigned long long hashJ = hashSubstringToI(j); //hash of S[1..j]
    
    return (hashJ - hashI) * power(cInv, i-1) % p;
}

void readInput(int argc, char **argv) {
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
    hashNonterminal();
    

    ////////// some unit tests //////////
    //test1: read from .plaintslp
    /* int j = 0;
    while (j<sizeRules) {
        printf("%u %u\n", R[j].left, R[j].right);
        j++;
    } */

    //test1.5: sizeNonTerminal
    printf ("size of nonterminals: \n");
    for (unsigned int i = 1; i <= sizeRules; i++) {
        printf ("%u %u\n", i, sizeN[i-1]);
    }

    //test2: comparing concate & hashN
    //print prime, c
    printf("prime: %llu\n", p);
    printf("c: %llu\n", c);
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
    for (unsigned int i = 1; i <= sizeRules; i++) {
        printf ("%u %llu\n", i, hashN[i-1]);
    }

    //test5: prefixB
    //unsigned long long *hashP = prefixB(e, 261);

    //test6.: reccurentPref
    /* unsigned long long r = recurrentPref(2, 261);
    printf("recurrentPref r: %llu\n", r);
    */
    //test7: hashSubstringToI
    unsigned int i = 3;
    unsigned long long hSub = hashSubstringToI(i);
    printf("hashSubstringToI i: %u hash: %llu\n", i, hSub);

    //test7.5: cInv
    unsigned long long result = (c*cInv) % p;
    printf("test7.5: %llu * %llu mod %llu should equal 1: %llu\n", c, cInv, p, result);

    //test8: hashSubstring S[i..j]
    unsigned long long hS = hashSubstring(2, 3);
    printf("test8 hash: %llu\n", hS);
    if (hS == getHash(256)){
        printf("test8: succeeded!\n");
    } else {
        printf("test8: retardo e stupido!\n");
    }

    //free(hashP);
    fclose(Pf);

}


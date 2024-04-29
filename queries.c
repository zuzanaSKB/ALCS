#include "grammar.h"

unsigned int *Blocksizes;
unsigned int *pattern;
unsigned int sizePattern;
unsigned int L;
unsigned int sizePrefHashTable;
unsigned int sizeSufHashTable;

uint64_t hashPatternBlock(unsigned int start, unsigned int end) {
    uint64_t hash = 0;
    uint64_t hash2 = 0;
    for (unsigned int i = start; i <= end; i++) {
        //hash += power(c, i) * fingerprint(pattern[i]) % p;
        //printf("left: %" PRIu64 " right: %" PRIu64 "\n", hash2, mul_mod_mersenne( power(c, i), fingerprint(pattern[i]), 61));
        hash2 = (hash2 + mul_mod_mersenne( power(c, i), fingerprint(pattern[i]), 61)) % p;
    }
    //printf("hash: %" PRIu64 " vs hash2: %" PRIu64 "\n", hash, hash2);
    return hash2 % p;
}

//binary search, finds k that is predecessor in Blocksizes of ⌈L/2⌉ == l
unsigned int findPredecessor(unsigned int l, unsigned int left, unsigned int right){
    while (left <= right) {
        unsigned int pivot = left + (right - left) / 2;
        //test purpose
        //printf("left: %u pivot: %u right: %u\n", left, pivot, right);
        if (Blocksizes[pivot] == l) {
            return pivot;
        }
        if (Blocksizes[pivot] < l) {
            if (Blocksizes[pivot+1] > l) {
                return pivot;
            }
            left = pivot;
        }
        else {
            right = pivot;
        }
    }
}

void querying(unsigned int sizeL, FILE *Resf) {
    unsigned int pos = 0;
    char c;
    unsigned int l = L / 2;
    if (L % 2 != 0) {
        l ++;
    }
    unsigned int k = 0;
    if (l > Blocksizes[sizeL-1]) {
        k = Blocksizes[sizeL-1];
    } else {
        k = Blocksizes[findPredecessor(l, 0, sizeL-1)];
    }

    //test findPredecessor
    printf("l: %u  k: %u\n", l, k);

    //check length of k
    if (k > sizePattern || k <= 0) {
        printf("Error! Wrong length of k.\n");
        exit(1);
    }

    uint64_t hashWindow = hashPatternBlock(0, k-1);
    //test purpose
    //uint64_t hashOfPattern = hashPatternBlock(0, sizePattern-1);
    //printf("hashOfPattern: %" PRIu64 "\n", hashOfPattern);

    for (int i = 0; i < sizePattern-k; i++) {
        //test purpose
        printf("PATTERN i: %u of :%u\n", i, sizePattern-k);
        printf("hashWindow: %" PRIu64 "\n", hashWindow);
        //if hash of k-window matches prefix hash block -> output l and position of its last character
        for (unsigned int b = 0; b < sizePrefHashTable; b++) {
            if (hashWindow == isPrefBlock[b].key) {
                c = 'l';
                pos = isPrefBlock[b].value + k - 1;
                fprintf(Resf, "%c %u\n", c, pos);
                //test purpose
                printf("%c %u\n", c, pos);
            }
        }
        //if hash of k-window matches suffix hash block -> output r and position of its first character
        for (unsigned int b = 0; b < sizeSufHashTable; b++) {
            if (hashWindow == isSufBlock[b].key) {
                c = 'r';
                pos = isSufBlock[b].value;
                fprintf(Resf, "%c %u\n", c, pos);
                //test purpose
                printf("%c %u\n", c, pos);
            }
        }
        //sliding window of length k updated in linear time
        hashWindow = (hashWindow - fingerprint(pattern[i])) % p;
        hashWindow = mul_mod_mersenne(hashWindow, cInv, 61);
        hashWindow = (hashWindow + mul_mod_mersenne(power(c, k-1), fingerprint(pattern[i+k]), 61)) % p;
    } 
    printf("Hash block matches has been written to the result file.\n");
}

unsigned int readIndexPatternL(int argc, char **argv) {
    FILE *Hf;
    char Hfname[1024];
    fputs("==== Command line: ====\n", stderr);
    for (int i = 0; i < argc; i++)
    fprintf(stderr, " %s", argv[i]);
    fputs("\n", stderr);
    if (argc != 4) {
        fprintf(stderr,
                "Usage: %s <index> <pattern> <L>\n"
                "This script read index from <Hfname>.hashtable, pattern from <filename>.txt \n"
                "and L, where L is approximately length of longest common substring\n"
                "and then perform queries on index. \n",
                argv[0]);
        exit(1);
    }
    // read <index>.hashtable file 
    strcpy(Hfname, argv[1]);
    strcat(Hfname, ".hashtable"); 
    Hf = fopen(Hfname, "rb");
    if (Hfname == NULL) {
        fprintf(stderr, "Error: cannot open file %s for reading\n", Hfname);
        exit(1);
    }
    unsigned int sizeL = 0;
    fread(&sizeL, sizeof(unsigned int), 1, Hf);
    Blocksizes = malloc(sizeL * sizeof(unsigned int));
    for(unsigned int i = 0; i < sizeL; i++){
        fread(&(Blocksizes[i]), sizeof(unsigned int), 1, Hf);
    }
    sizePrefHashTable = 0;
    sizeSufHashTable = 0;
    fread(&sizePrefHashTable, sizeof(unsigned int), 1, Hf);
    fread(&sizeSufHashTable, sizeof(unsigned int), 1, Hf);
    isPrefBlock = malloc(sizePrefHashTable * sizeof(THashPair));
    isSufBlock = malloc(sizeSufHashTable * sizeof(THashPair));
    for(unsigned int i = 0; i < sizePrefHashTable; i++){
        fread(&(isPrefBlock[i].key), sizeof(uint64_t), 1, Hf);
        fread(&(isPrefBlock[i].value), sizeof(unsigned int), 1, Hf);
    }
    for(unsigned int i = 0; i < sizeSufHashTable; i++){
        fread(&(isSufBlock[i].key), sizeof(uint64_t), 1, Hf);
        fread(&(isSufBlock[i].value), sizeof(unsigned int), 1, Hf);
    }
    printf("Index loaded.\n");

    //test1 : readHf - read sizeL, hashtable
    /* printf("sizeL: %u\n", sizeL);
    for (unsigned int i = 0; i < sizeL; i++) {        
        printf("Blocksizes: i: %u  value: %u\n", i, Blocksizes[i]);
    }
    printf("sizePrefHashTable: %u  sizeSufHashTable: %u\n", sizePrefHashTable, sizeSufHashTable);
    for (unsigned int i = 0; i < sizePrefHashTable; i++) {        
        printf("PrefixB: i: %u  key: %" PRIu64 "  value: %u\n", i, isPrefBlock[i].key, isPrefBlock[i].value);
    }
    for (unsigned int i = 0; i < sizeSufHashTable; i++) {        
        printf("SufixB: i: %u  key: %" PRIu64 "  value: %u\n", i, isSufBlock[i].key, isSufBlock[i].value);
    } */

    //read <pattern>.txt file
    FILE *Pf;
    char Pfname[1024];
    strcpy(Pfname, argv[2]);
    strcat(Pfname, ".txt"); 
    Pf = fopen(Pfname, "r");
    if (Pf == NULL) {
        fprintf(stderr, "Error: cannot open file %s for reading\n", Pfname);
        exit(1);
    }
    sizePattern = 0;
    int c;
    while ((c = fgetc(Pf)) != EOF) {
        sizePattern ++;
    }
    //sizePattern--;
    // rewind file pointer
    rewind(Pf);
    // allocate array of rules
    pattern = malloc(sizePattern * sizeof(unsigned int));
    if (pattern == NULL) {
        printf("Error: memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < sizePattern; i++) {
        c = fgetc(Pf);
        pattern[i] = c;
    }
    printf("Pattern loaded.\n");
    //test2 : read pattern
    /* printf("sizeOfPattern: %u\n", sizePattern);
    printf("pattern: \n");
    for (int i = 0; i < sizePattern; i++) {
        printf("%u ", pattern[i]);
    }
    printf("\n"); */

     
    //read L
    L = 0;
    L = atoi(argv[3]);
    printf("loaded L : %u\n", L);

    fclose(Pf);
    fclose(Hf);

    return sizeL; //size of L Blocks
}

int main(int argc, char **argv) {
    unsigned int sizeL = readIndexPatternL(argc, argv);
    FILE *Resf;
    char resfname[1024];
    strcpy(resfname, argv[2]);
    strcat(resfname, ".resf");
    Resf = fopen(resfname, "w");
    
    querying(sizeL, Resf);

    fclose(Resf);

    free(isPrefBlock);
    free(isSufBlock);
    free(pattern);
    free(Blocksizes);

    return 0;
}
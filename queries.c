#include "grammar.h"

unsigned int *Blocksizes;
unsigned int *pattern;
unsigned int sizePattern;
unsigned int L;

uint64_t hashPatternBlock(unsigned int start, unsigned int end) {
    uint64_t hash = 0;
    for (unsigned int i = start; i <= end; i++) {
        hash += power(c, i+1) * fingerprint(pattern[i]) % p;
    }
    return hash;
}

//binary search k that is predecessor in Blocksizes of ⌈L/2⌉ == l
unsigned int findPredecessor(unsigned int l, unsigned int left, unsigned int right){
    while (left <= right) {
        unsigned int pivot = left + (right - left) / 2;
        //test
        printf("left: %u pivot: %u right: %u\n", left, pivot, right);
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

void querying(unsigned int sizeHashTable) {
    unsigned int pos = 0;
    char c;
    unsigned int l = L / 2;
    if (L % 2 != 0) {
        l ++;
    }
    unsigned int k = findPredecessor(l, 0, sizeHashTable-1);

    //test findPredecessor
    printf("l: %u  k: %u\n", l, k);

    for (unsigned int i = 0; i < sizePattern-k; i++) {
        if (hashPatternBlock(i, i+k-1) == isPrefBlock[k].key){
            c = 'l';
            pos = i+k-1;
        }
        if (hashPatternBlock(i, i+k-1) == isSufBlock[k].key){
            c = 'r';
            pos = i;
        }
    }



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
                "Usage: %s <filename> <e>\n"
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
    unsigned int sizeHashTable = 0;
    fread(&sizeHashTable, sizeof(unsigned int), 1, Hf);
    Blocksizes = malloc(sizeHashTable * sizeof(unsigned int));
    isPrefBlock = malloc(sizeHashTable * sizeof(THashPair));
    isSufBlock = malloc(sizeHashTable * sizeof(THashPair));
    for(unsigned int i = 0; i < sizeHashTable; i++){
        fread(&(Blocksizes[i]), sizeof(unsigned int), 1, Hf);
    }
    for(unsigned int i = 0; i < sizeHashTable; i++){
        fread(&(isPrefBlock[i].key), sizeof(uint64_t), 1, Hf);
        fread(&(isPrefBlock[i].value), sizeof(unsigned int), 1, Hf);
    }
    for(unsigned int i = 0; i < sizeHashTable; i++){
        fread(&(isSufBlock[i].key), sizeof(uint64_t), 1, Hf);
        fread(&(isSufBlock[i].value), sizeof(unsigned int), 1, Hf);
    }

    //test1 : readHf
    printf("sizeHashTable: %u\n", sizeHashTable);
    for (unsigned int i = 0; i < sizeHashTable; i++) {        
        printf("Blocksizes: i: %u  value: %u\n", i, Blocksizes[i]);
    }
    for (unsigned int i = 0; i < sizeHashTable; i++) {        
        printf("PrefixB: i: %u  key: %" PRIu64 "  value: %u\n", i, isPrefBlock[i].key, isPrefBlock[i].value);
    }
    for (unsigned int i = 0; i < sizeHashTable; i++) {        
        printf("SufixB: i: %u  key: %" PRIu64 "  value: %u\n", i, isSufBlock[i].key, isSufBlock[i].value);
    }

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
    sizePattern--;
    //testpurpose
    printf("sizePattern: %u\n", sizePattern);
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

    //test2 : read pattern
    printf("pattern: \n");
    for (int i = 0; i < sizePattern; i++) {
        printf("%u ", pattern[i]);
    }
    printf("\n");

     
    //read L
    L = 0;
    L = atoi(argv[3]);
    printf("loaded L : %u\n", L);

    fclose(Pf);
    fclose(Hf);

    return sizeHashTable; //size of L Blocks
}

int main(int argc, char **argv) {
    unsigned int sizeHashTable = readIndexPatternL(argc, argv);
    
    querying(sizeHashTable);

    free(isPrefBlock);
    free(isSufBlock);
    free(pattern);
    free(Blocksizes);

    return 0;
}
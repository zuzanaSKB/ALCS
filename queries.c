#include "grammar.h"

unsigned int *Blocksizes;
unsigned int *pattern;
unsigned int sizePattern;
unsigned int L;
unsigned int sizeHashTable;

uint64_t hashPatternBlock(unsigned int start, unsigned int end) {
    uint64_t hash = 0;
    for (unsigned int i = start; i <= end; i++) {
        hash += power(c, i) * fingerprint(pattern[i]) % p;
    }
    return hash;
}

//binary search, finds k that is predecessor in Blocksizes of ⌈L/2⌉ == l
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

void querying(unsigned int sizeL, FILE *Resf) {
    unsigned int pos = 0;
    char c;
    unsigned int l = L / 2;
    if (L % 2 != 0) {
        l ++;
    }
    unsigned int k = findPredecessor(l, 0, sizeL-1);

    //test findPredecessor
    printf("l: %u  k: %u\n", l, k);

    uint64_t hash1ToK = hashPatternBlock(0, k-1);
    printf("hash1ToK: %" PRIu64 "\n", hash1ToK);
    for (unsigned int b = 0; b < sizeHashTable; b++) {
        if (hash1ToK == isPrefBlock[b].key) {
            c = 'l';
            pos = k-1;
            fprintf(Resf, "%c %u\n", c, pos);
        }
        if (hash1ToK == isSufBlock[b].key) {
            c = 'r';
            pos = 0;
            fprintf(Resf, "%c %u\n", c, pos);
        }
    }
    
    /* for (int i = 0; i < sizePattern-k; i++) {

    } */

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
    unsigned int sizeL = 0;
    fread(&sizeL, sizeof(unsigned int), 1, Hf);
    Blocksizes = malloc(sizeL * sizeof(unsigned int));
    for(unsigned int i = 0; i < sizeL; i++){
        fread(&(Blocksizes[i]), sizeof(unsigned int), 1, Hf);
    }
    sizeHashTable = 0;
    fread(&sizeHashTable, sizeof(unsigned int), 1, Hf);
    isPrefBlock = malloc(sizeHashTable * sizeof(THashPair));
    isSufBlock = malloc(sizeHashTable * sizeof(THashPair));
    for(unsigned int i = 0; i < sizeHashTable; i++){
        fread(&(isPrefBlock[i].key), sizeof(uint64_t), 1, Hf);
        fread(&(isPrefBlock[i].value), sizeof(unsigned int), 1, Hf);
    }
    for(unsigned int i = 0; i < sizeHashTable; i++){
        fread(&(isSufBlock[i].key), sizeof(uint64_t), 1, Hf);
        fread(&(isSufBlock[i].value), sizeof(unsigned int), 1, Hf);
    }

    //test1 : readHf
    printf("sizeL: %u\n", sizeL);
    for (unsigned int i = 0; i < sizeL; i++) {        
        printf("Blocksizes: i: %u  value: %u\n", i, Blocksizes[i]);
    }
    printf("sizeHashTable: %u\n", sizeHashTable);
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

    return sizeL; //size of L Blocks
}

int main(int argc, char **argv) {
    unsigned int sizeL = readIndexPatternL(argc, argv);
    FILE *Resf;
    Resf = fopen("ResF.txt", "w");
    
    querying(sizeL, Resf);

    fclose(Resf);

    free(isPrefBlock);
    free(isSufBlock);
    free(pattern);
    free(Blocksizes);

    return 0;
}
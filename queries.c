#include "grammar.h"

unsigned int *pattern;
unsigned int sizePattern;
unsigned int L;
//THashPair *isPrefBlock;
//THashPair *isSufBlock;

uint64_t hashPattern() {
    uint64_t hashOfPattern = 0;
    for (unsigned int i = 0; i < sizePattern; i++) {
        hashOfPattern += power(c, i+1) * fingerprint(pattern[i]) % p;
    }
    return hashOfPattern;
}

/* void querying() { TO DO
    unsigned int Lhalf = L / 2;
    if (L % 2 != 0) {
        Lhalf ++;
    }

} */

void readIndexPatternL(int argc, char **argv) {
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
    unsigned int capacity = 1;
    THashPair tempBlock;
    isPrefBlock = malloc(capacity * sizeof(THashPair));
    while (fread(&(tempBlock.key), sizeof(uint64_t), 1, Hf) == 1 &&
           fread(&(tempBlock.value), sizeof(unsigned int), 1, Hf) == 1) {
        // Check if more memory needs to be allocated
        if (sizeHashTable >= capacity) {
            capacity *= 2;
            isPrefBlock = realloc(isPrefBlock, capacity * sizeof(THashPair));
            if (isPrefBlock == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                fclose(Hf);
                exit(1);
            }
        }
        // Copy the read data into the dynamically allocated array
        isPrefBlock[sizeHashTable] = tempBlock;
        sizeHashTable++;
    }

    //test1 : readHf
    for (unsigned int i = 0; i < sizeHashTable; i++) {
        printf("PrefixB: i: %u  key: %" PRIu64 "  value: %u\n", i, isPrefBlock[i].key, isPrefBlock[i].value);
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
}

int main(int argc, char **argv) {
    readIndexPatternL(argc, argv);
    uint64_t hashOfPattern = hashPattern();
    printf("hash of pattern: %" PRIu64 "\n", hashOfPattern);
    //querying();

    free(isPrefBlock);
    //free(isSufBlock);
    free(pattern);
    return 0;
}
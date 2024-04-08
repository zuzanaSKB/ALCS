#include "grammar.h"

//computes hashes of pref and suf blocks
//creates hash table and writes in on disk
void buildIndex(float e) {
    unsigned int sizeText = getSize(sizeRules+offset-1);
    //compute L
    unsigned int sizeL = 1;
    unsigned int *L;
    L = (unsigned int *)malloc(1 * sizeof(unsigned int));
    L[0] = 1;
    printf("hashBlock:\n");

    for (unsigned int i = 2; i < sizeText; i++) {
        if (i >= L[sizeL-1]*(1/(1-e))) {
            sizeL ++;
            L = (unsigned int *)realloc(L, sizeL * sizeof(unsigned int));
            L[sizeL - 1] = i;
            printf("L[%u] = %u\n", i, L[sizeL - 1]);
        }
    }

    //prefix
    isPrefBlock = (void *)malloc(sizeL * sizeof(THashPair));
    for(unsigned int i = 0; i < sizeL; i++){

        isPrefBlock[i].key = hashSubstring(1, L[i]);
        isPrefBlock[i].value = 1;

        //test purpose
        printf("PrefixB: i: %u  L[i]: %u key: %" PRIu64 "\n", i, L[i], isPrefBlock[i].key);
    }

    //sufix
    isSufBlock = (void *)malloc(sizeL * sizeof(THashPair));
    for(unsigned int i = 0; i < sizeL; i++){

        //test purpose
        printf("Sufix: i: %u  L[i]: %u\n", i, L[i]);

        isSufBlock[i].key = hashSubstring(sizeText - L[i], sizeText-1);
        isSufBlock[i].value = sizeText - L[i];
    }

    ///////////////// save index to disk /////////////////
    FILE *Hf;
    char output[1024];
    strcpy(output, "index");
    strcat(output, ".hashtable"); 
    Hf = fopen(output, "wb");
    for(unsigned int i = 0; i < sizeL; i++){
        fwrite(&(isPrefBlock[i].key), sizeof(uint64_t), 1, Hf);
        fwrite(&(isPrefBlock[i].value), sizeof(unsigned int), 1, Hf);
    }
    
    fclose(Hf);
    printf("Built index has been written to %s\n", output);

}

int main(int argc, char **argv) {
    readInput(argc,argv); 
    sizeNonTerminal();
    hashNonterminal();
    //array with first occurencies of exp(X) of all terminals and nonterminals
    indicesOfExpX = (void *)calloc((sizeRules+offset), sizeof(unsigned int));
    computeIndicesOfExpX(sizeRules + offset-1, 1);

    buildIndex(e);
    
    //free memory
    free(R);
    free(isPrefBlock);
    free(isSufBlock);
    free(sizeN);
    free(hashN);
    free(indicesOfExpX);

    return 0;
}


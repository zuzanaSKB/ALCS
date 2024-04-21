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

    //prefix and sufix blocks
    isPrefBlock = (void *)malloc(sizeRules * sizeL * sizeof(THashPair));
    isSufBlock = (void *)malloc(sizeRules * sizeL * sizeof(THashPair));
    unsigned int isPrefBlocki = 0;
    unsigned int isSufBlocki = 0;
    for (int x = 0; x < sizeRules; x++) {
        unsigned int x_size = getSize(x+offset);
        unsigned int l_size = getSize(R[x].left);
        unsigned int r_size = getSize(R[x].right);
        for(unsigned int i = 0; i < sizeL; i++){
            //printf("getSize(x+offset): %u  L[i]: %u\n",getSize(x+offset), L[i]);
            //printf("i: %u\n", i);
            if (x_size >= L[i]) {
                if (l_size < L[i]) {
                    isPrefBlock[isPrefBlocki].key = hashSubstring(indicesOfExpX[x+offset], indicesOfExpX[x+offset] + L[i] - 1);
                    isPrefBlock[isPrefBlocki].value = indicesOfExpX[x+offset];
                    
                    //test purpose
                    //printf("i: %u  j: %u  hash:%" PRIu64 "\n", indicesOfExpX[x+offset], indicesOfExpX[x+offset] + L[i] - 1, isPrefBlock[isPrefBlocki].key);
                    //printf("isPrefBlock[%u]: key: %" PRIu64 " value: %u\n", isPrefBlocki, isPrefBlock[isPrefBlocki].key, isPrefBlock[isPrefBlocki].value);

                    isPrefBlocki++; 
                }
                if(r_size < L[i]) {
                    isSufBlock[isSufBlocki].key = hashSubstring(indicesOfExpX[x+offset] + getSize(x+offset) - L[i], indicesOfExpX[x+offset] + getSize(x+offset) - 1);
                    isSufBlock[isSufBlocki].value = indicesOfExpX[x+offset] + getSize(x+offset) - L[i];

                    //test purpose
                    //printf("isSufBlock[%u]: key: %" PRIu64 " value: %u\n", isSufBlocki, isSufBlock[isSufBlocki].key, isSufBlock[isSufBlocki].value);

                    isSufBlocki++; 
                }
                              
            } else {
                break;
            }
        }
        //test
        printf("processed %u of %u\n", (x+1)*sizeL, sizeRules*sizeL);
    }

    ///////////////// save index to disk /////////////////
    FILE *Hf;
    char output[1024];
    strcpy(output, "index");
    strcat(output, ".hashtable"); 
    Hf = fopen(output, "wb");
    fwrite(&sizeL, sizeof(unsigned int), 1, Hf);
    for(unsigned int i = 0; i < sizeL; i++){
        fwrite(&(L[i]), sizeof(unsigned int), 1, Hf);
    }
    fwrite(&isPrefBlocki, sizeof(unsigned int), 1, Hf);
    fwrite(&isSufBlocki, sizeof(unsigned int), 1, Hf);
    for(unsigned int i = 0; i < isPrefBlocki; i++){
        fwrite(&(isPrefBlock[i].key), sizeof(uint64_t), 1, Hf);
        fwrite(&(isPrefBlock[i].value), sizeof(unsigned int), 1, Hf);
    }
    for(unsigned int i = 0; i < isSufBlocki; i++){
        fwrite(&(isSufBlock[i].key), sizeof(uint64_t), 1, Hf);
        fwrite(&(isSufBlock[i].value), sizeof(unsigned int), 1, Hf);
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
    //test9: computeIndicesOfExpX
    FILE *Lf;
    Lf = fopen("logfile.txt", "w");
    fprintf(Lf, "test9: computeIndicesOfExpX\n");
    for(unsigned int i = 0; i < sizeRules+offset; i++) {
        if(indicesOfExpX[i] != 0) {
            fprintf(Lf, "start of exp(%u) : %u\n", i, indicesOfExpX[i]);
        }
    }
    fclose(Lf);
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


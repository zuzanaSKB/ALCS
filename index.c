#include "grammar.h"

// computes hashes of pref and suf blocks
// creates hash table and writes in on disk
void buildIndex(float e) {
    unsigned int sizeText = getSize(sizeRules+offset-1);
    //compute L - lengths of blocks
    unsigned int sizeL = 1;
    unsigned int *L;
    L = (unsigned int *)malloc(1 * sizeof(unsigned int));
    L[0] = 1;
    for (unsigned int i = 2; i < sizeText; i++) {
        if (i >= L[sizeL-1]*(1/(1-e))) {
            sizeL ++;
            L = (unsigned int *)realloc(L, sizeL * sizeof(unsigned int));
            L[sizeL - 1] = i;
        }
    }

    //prefix and suffix blocks
    isPrefBlock = (void *)malloc(sizeRules * sizeL * sizeof(THashPair));
    isSufBlock = (void *)malloc(sizeRules * sizeL * sizeof(THashPair));
    unsigned int isPrefBlocki = 0;
    unsigned int isSufBlocki = 0;
    for (int x = 0; x < sizeRules; x++) {
        unsigned int x_size = getSize(x+offset);
        unsigned int l_size = getSize(R[x].left);
        unsigned int r_size = getSize(R[x].right);
        for(unsigned int i = 0; i < sizeL; i++){
            if (x_size >= L[i]) {
                unsigned int startExpX = indicesOfExpX[x+offset];

                if (l_size <= L[i]) {
                    isPrefBlock[isPrefBlocki].key = hashSubstring(startExpX, startExpX + L[i] - 1);
                    isPrefBlock[isPrefBlocki].value = startExpX;
                    isPrefBlocki++; 
                }
                if(r_size <= L[i]) {
                    isSufBlock[isSufBlocki].key = hashSubstring(startExpX + x_size - L[i], startExpX + x_size - 1);
                    isSufBlock[isSufBlocki].value = startExpX + x_size - L[i];
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

    printf("number of blocks: %u\n", isPrefBlocki+isSufBlocki);

}

int main(int argc, char **argv) {
    readInput(argc,argv); 
    sizeNonTerminal();
    hashNonterminal();
    
    //array with first occurencies of exp(X) of all terminals and nonterminals
    indicesOfExpX = (void *)calloc((sizeRules+offset), sizeof(unsigned int));
    computeIndicesOfExpX(sizeRules + offset-1, 1);
    buildIndex(e);
    
    //test purpose save to logfile
    /* FILE *Lf;
    Lf = fopen("logfile.txt", "w");
    fprintf(Lf, "test9: computeIndicesOfExpX\n");
    for(unsigned int i = 0; i < sizeRules+offset; i++) {
        if(indicesOfExpX[i] != 0) {
            fprintf(Lf, "start of exp(%u) : %u\n", i, indicesOfExpX[i]);
        }
    }
    fclose(Lf); */
    
     
    //free memory
    free(R);
    free(isPrefBlock);
    free(isSufBlock);
    free(sizeN);
    free(hashN);
    free(indicesOfExpX);

    return 0;
}


#include "grammar.h"

//computes hashes of pref and suf blocks
//creates hash table and writes in on disk
void buildIndex(float e) {
    unsigned int sizeText = getSize(sizeRules+offset-1);
    //compute L
    FILE *Lf;
    char logfile [1024];
    strcpy(logfile, output);
    strcat(logfile, ".logfile");
    Lf = fopen(logfile, "w");
    unsigned int sizeL = 1;
    unsigned int *L;
    L = (unsigned int *)malloc(1 * sizeof(unsigned int));
    L[0] = 1;
    //printf("hashBlock:\n");
    //printf("L[0] = 1\n");
    fprintf(Lf, "hashBlocks:\n");
    fprintf(Lf, "L[0] = 1\n");
    for (unsigned int i = 2; i < sizeText; i++) {
        if (i >= L[sizeL-1]*(1/(1-e))) {
            sizeL ++;
            L = (unsigned int *)realloc(L, sizeL * sizeof(unsigned int));
            L[sizeL - 1] = i;
            //printf("L[%u] = %u\n", sizeL-1, L[sizeL - 1]);
            fprintf(Lf, "L[%u] = %u\n", sizeL-1, L[sizeL - 1]);

        }
    }
    fprintf(Lf, "number of blocks: %u", sizeL);
    fclose(Lf);

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
                unsigned int startExpX = indicesOfExpX[x+offset];
                //printf("startExpX: %u\n", startExpX);

                if (l_size <= L[i]) {
                    isPrefBlock[isPrefBlocki].key = hashSubstring(startExpX, startExpX + L[i] - 1);
                    isPrefBlock[isPrefBlocki].value = startExpX;
                    
                    //test purpose
                    //printf("i: %u  j: %u  hash:%" PRIu64 "\n", startExpX, startExpX + L[i] - 1, isPrefBlock[isPrefBlocki].key);
                    //printf("isPrefBlock[%u]: key: %" PRIu64 " value: %u\n", isPrefBlocki, isPrefBlock[isPrefBlocki].key, isPrefBlock[isPrefBlocki].value);

                    isPrefBlocki++; 
                }
                if(r_size <= L[i]) {
                    isSufBlock[isSufBlocki].key = hashSubstring(startExpX + x_size - L[i], startExpX + x_size - 1);
                    isSufBlock[isSufBlocki].value = startExpX + x_size - L[i];

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
    //test purpose
    FILE *Tf;
    //strcpy(output, "index");
    strcat(output, ".hashtable");
    Hf = fopen(output, "wb");
    //test purpose
    strcat(output, ".txt");
    Tf = fopen(output, "w");
    fwrite(&sizeL, sizeof(unsigned int), 1, Hf);
    for(unsigned int i = 0; i < sizeL; i++){
        fwrite(&(L[i]), sizeof(unsigned int), 1, Hf);
    }
    fwrite(&isPrefBlocki, sizeof(unsigned int), 1, Hf);
    fwrite(&isSufBlocki, sizeof(unsigned int), 1, Hf);
    //test purpose
    fprintf(Tf, "%u\n",isPrefBlocki);
    fprintf(Tf, "%u\n",isSufBlocki);

    for(unsigned int i = 0; i < isPrefBlocki; i++){
        fwrite(&(isPrefBlock[i].key), sizeof(uint64_t), 1, Hf);
        fwrite(&(isPrefBlock[i].value), sizeof(unsigned int), 1, Hf);
        //test purpose
        fprintf(Tf,"%" PRIu64 "\n",isPrefBlock[i].key);
        fprintf(Tf, "%u\n",isPrefBlock[i].value);
    }
    for(unsigned int i = 0; i < isSufBlocki; i++){
        fwrite(&(isSufBlock[i].key), sizeof(uint64_t), 1, Hf);
        fwrite(&(isSufBlock[i].value), sizeof(unsigned int), 1, Hf);
        //test purpose
        fprintf(Tf,"%" PRIu64 "\n",isSufBlock[i].key);
        fprintf(Tf, "%u\n",isSufBlock[i].value);
    }
    
    fclose(Hf);
    //test purpose
    fclose(Tf);
    printf("Built index has been written to %s\n", output);

}

int main(int argc, char **argv) {
    readInput(argc,argv); 
    sizeNonTerminal();
    hashNonterminal();

    //test1.5: sizeNonTerminal
    /* printf ("size of nonterminals: \n");
    for (unsigned int i = 0; i < sizeRules; i++) {
        printf ("%u %u\n", i+offset, sizeN[i]);
    } */

    //test3: print all hashes of nontermonals
    /* printf ("hashes of nonterminals: \n");
    for (unsigned int i = 0; i < sizeRules; i++) {
        printf ("%u %" PRIu64 "\n", i+offset, hashN[i]);
    } */
    
    //array with first occurencies of exp(X) of all terminals and nonterminals
    indicesOfExpX = (void *)calloc((sizeRules+offset), sizeof(unsigned int));
    computeIndicesOfExpX(sizeRules + offset-1, 1);
    buildIndex(e);
    
    //test purpose saved to logfile
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


#include "grammar.h"

int main(int argc, char **argv) {
    readInput(argc,argv); 

    
    //free memory
    free(R);
    free(sizeN);
    free(hashN);
    free(indicesOfExpX);
    return 0;
}


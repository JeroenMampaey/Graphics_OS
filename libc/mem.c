#include "mem.h"

void memory_copy(char *source, char *dest, int nbytes) {
    for (int i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

void memory_clear(char* source, int nbytes){
    for(int i=0; i<nbytes; i++){
        *(source + i) = 0;
    }
}
#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

#define NITEMS 1024

int usage(char *prog){
    printf("Usage is %s <file_name>\n", prog);
    printf("\n");
    printf("Read ther raw binary cmp_t file and print\n");
    printf("Its values to stdout in floating poiont decimal\n");
    return 0;
}

int main(int argc, char **argv){
    cmp_t *wave;
    char *file_name;
    int tsize;
    int nitems;
    int i;
    int n;

    if(argc < 2) {
        usage(argv[0]);
        return 0;
    }
    file_name = argv[1];
    n = nsize(file_name);
    if(n<0) return 1;
    printf("File %s contains %d entries\n", file_name, n);
    tsize = sizeof(cmp_t)*n;
    wave = (cmp_t *)malloc(tsize);
    if(wave == NULL){
        printf("Error allocating %d bytes for wave table\n", tsize);
        return 0;
    }
    nitems = dft_fread(file_name, wave, n);
    printf("read %d items from file %s\n", nitems, file_name);
    for(i=0; i<nitems;i++){
        printf("wav[%7i] = %.7f, %.7f\n", i, wave[i].r, wave[i].i);
    }
    free(wave);
}

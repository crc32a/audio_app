#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("usage is %s <output file name> ", prog);
    printf("<input file name> <scale>\n", prog);
    printf("\n");
    printf("scale the signal given by the input ");
    printf("by the scale factor\n");
    return 0;
}

int main(int argc, char **argv){
    char *fi_name;
    char *fo_name;
    cmp_t *in;
    cmp_t *out;
    double scale;
    int tsize;
    int i;
    int n;
    if(argc <3){
       usage(argv[0]);
       return 0;
    }
    fo_name = argv[1];
    fi_name = argv[2];
    scale = atof(argv[3]);
    printf("input file: %s\n", fi_name);
    printf("output file: %s\n", fo_name);
    printf("scale factor: %f\n", scale);
    n = nsize(fi_name);
    if(n < 0){
        printf("Error getting entry count for file %s\n", fi_name);
        return -1;
    }
    printf("Expecting %d sasmples from input file\n", n);
    tsize = sizeof(cmp_t)*n;
    in = (cmp_t *)malloc(tsize);
    if(in == NULL){
        printf("Error failed to allocate %d ", tsize);
        printf(" bytes for input\n", tsize);
        return -1;
    }
    out = (cmp_t *)malloc(tsize);
    if(out == NULL){
        printf("Error failed to allocate %d ", tsize);
        printf(" bytes for input\n", tsize);
        free(in);
        return -1;
    }
    printf("Reading input file %s now\n", fi_name);
    dft_fread(fi_name, in, n);
    for(i=0;i<n;i++){
        out[i].r = scale*in[i].r;
        out[i].i = scale*in[i].i;
    }
    printf("Writeing output file %s now\n", fo_name);
    dft_fwrite(fo_name, out, n);
    free(in);
    free(out);
    return 0;
    
}

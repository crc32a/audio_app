#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("Usage is %s <file_name_out> <file_name_out> ", prog);
    printf("<freq> <sr> <q>\n");
    printf("\n");
    printf("Apply the biquad filter with the parameters listed\n");
    printf("Above.\n");
    return 0;
}

int main(int argc, char **argv){
    double *a;
    double *b;
    double q;
    char *fi_name;
    char *fo_name;
    cmp_t *x;
    cmp_t *y;
    double f;
    int tsize;
    int resp;
    int sr;
    int nc;
    int n;
    int i;

    if(argc < 6){
        usage(argv[0]);
        return 0;
    }

    fo_name = argv[1];
    fi_name = argv[2];
    f = atof(argv[3]);
    sr = atoi(argv[4]);
    q = atof(argv[5]);

    printf("filter parameters\n");
    printf("-----------------------------------------\n");
    printf("input file:   %s\n", fi_name);
    printf("output file:  %s\n", fo_name);
    printf("freqiency:    %f\n", f);
    printf("sample rate:  %d\n", sr);
    printf("q parameter:  %f\n", q);
    printf("\n");
    n = nsize(fi_name);
    if(n<0){
        printf("Error reading input file %s\n", fi_name);
        return 0;
    }
    printf("reading &d samples from input\n", n);
    nc = biquadcoeff(&a, &b, sr, f, q);
    if(nc != 3){
        printf("Error calculating coeffients for filter\n");
        return 0;
    }
    printf("Coefficiants calculaterd\n");
    printf("---------------------------------\n");
    for(i=0;i<3;i++){
        printf("a[%i] = %f, b[%i] = %f\n",i,a[i],i,b[i]);
    }
    tsize = sizeof(cmp_t)*n;
    x = (cmp_t *)malloc(tsize);
    if(x == NULL){
        printf("Failed to allocate %d bytes for ", tsize);
        printf("input buffer\n");
        free(a);
        free(b);
        return -1;
    }
    y = (cmp_t *)malloc(tsize);
    if(y == NULL){
        printf("Failed to allocate %d bytes for ", tsize);
        printf("output buffer\n");
        free(a);
        free(b);
        free(x);
        return -1;
    }
    printf("Reading %s file for input\n", fi_name);
    resp = dft_fread(fi_name, x, n);
    if(resp < n){
        printf("Error reading file %s: ", fi_name);
        perror("");
        free(a);
        free(b);
        free(x);
        free(y);
        return -1;
    }
    printf("Applying filter\n");
    resp = biquadfilter(y, x, a, b, 3, n);
    printf("writeing filtered data to %s\n", fo_name);
    resp = dft_fwrite(fo_name, y, n);
    if(resp < n){
        printf("Error writing to %s: ", fo_name);
        perror("");
        free(a);
        free(b);
        free(x);
        free(y);
        return -1;
    }
    free(a);
    free(b);
    free(x);
    free(y);
    return 0;
}

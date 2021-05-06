#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("Usage is %s ");
    printf("<file_name> <amp> <freq> <phase> <sr> <secs>\n", prog);
    printf("\n");
    printf("Generate a square wave with the freq amplitude phase\n");
    printf("sample rate and length of time in seconds above\n");
    printf("and store the results in the file specified above\n");
    return 0;
}

int main(int argc, char **argv){
    cmp_t *wave;
    char *fmt;
    char *file_name;
    double a;
    double f;
    double ph;
    double sr;
    double secs;
    int n;
    int tsize;
    if(argc<7){
        usage(argv[0]);
        return 0;
    }
    file_name = argv[1];
    a = atof(argv[2]);
    f = atof(argv[3]);
    ph = atof(argv[4]);
    sr = atof(argv[5]);
    secs = atof(argv[6]);
    n = (int)(sr*secs);
    tsize = sizeof(cmp_t)*n;
    wave = (cmp_t *)malloc(tsize);
    if(wave == NULL){
        printf("Error allocating %d bytes for wave table\n", tsize);
        return 0;
    }
    printf("Computing sinewave for \n");
    printf("-----------------------\n");
    printf("file name    %s\n", file_name);
    printf("amplitude    %f\n", a);
    printf("frequency    %f\n", f);
    printf("phase        %f\n", ph);
    printf("Sample rate  %f\n", sr);
    printf("length(secs) %f\n", secs);
    printf("\n");
    sigsquare(wave, a, f, ph, sr, n);
    printf("Computation done writing %d entres file\n", n);
    dft_fwrite(file_name, wave, n);
    printf("Data written closing file\n");
    free(wave);
    return 0;
}

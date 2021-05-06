#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("Usage is &s <file_out> <file_in> <sample_rate>\n", prog);
    printf("\n");
    printf("Convert the input datfile to a wav playable file\n");
    return 0;
}

int main(int  argc, char **argv){
    char *fi_name;
    char *fo_name;
    cmp_t *data;
    int sr;
    int nread;
    int n;
    int status;
    int tsize;
    if(argc<4){
        usage(argv[0]);
        return -1;
    }
    fo_name = argv[1];
    fi_name = argv[2];
    sr = atoi(argv[3]);
    printf("Reading from %s ", fi_name);
    printf("and converting to wave file %s\n", fo_name);
    printf("with a sample rate of %d\n", sr);
    n = nsize(fi_name);
    if(n < 0){
        printf("Error getting entry count from file %s\n", fi_name);
        return -1;
    }
    printf("Reading %d entries from file %s\n", n, fi_name);
    tsize = sizeof(cmp_t)*n;
    data = (cmp_t *)malloc(tsize);
    if(data == NULL){
        printf("Error allocating %d bytes for wave table\n");
        return -1;
    }
    nread = dft_fread(fi_name, data, n);
    if(nread < n){
        printf("Error reading input file %s\n", fi_name);
        perror("");
        free(data);
        return 0;
    }
    status = fwrite_wav(fo_name, data, sr, n);
    printf("Status of write opertaion is is %d\n", status);
    if(status<0){
        printf("Error writing to output file %s\n", fo_name);
    };
    free(data);
    return 0;
}

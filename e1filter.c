#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("Usage is %s <file_out> <file_out>\n", prog);
    printf("\n");
    printf("apply the e1 filter to the file specified above\n");
    return 0;
}

int main(int argc, char **argv){
    char *fo_name;
    char *fi_name;
    cmp_t *x;
    cmp_t *y;
    int tsize;
    int resp;
    int n;
    
    if(argc<3){
        usage(argv[0]);
        return 0;
    }
    fo_name = argv[1];
    fi_name = argv[2];

    n = nsize(fi_name);
    if(n<0){
        printf("Error reading from file %s\n", fi_name);
        return 0;
    }
    printf("Will read %d entries from %s\n", n, fi_name);
    tsize = sizeof(cmp_t) * n;
    x = (cmp_t *)malloc(tsize);
    if(x == NULL){
        printf("Error allocating %d bytes ", tsize);
        printf("for input buffer\n");
        return -1;
    }
    y = (cmp_t *)malloc(tsize);
    if(y == NULL){
        printf("Error allocating %d bytes ", tsize);
        printf("for output buffer\n");
        free(x);
        return -1;
    }
    printf("Reading data now\n");
    resp = dft_fread(fi_name, x, n);
    if(resp < n){
        printf("Error reading from file %s\n", fi_name);
        free(x);
        free(y);
        return -1;
    }
    printf("Applying e1 filter\n");
    resp = e1filter(y, x, n);
    if(resp != 0){
        printf("Error applying filter to data\n");
        free(x);
        free(y);
        return -1;
    }
    printf("Writing filtered data to output file ");
    printf("%s\n", fo_name);
    resp = dft_fwrite(fo_name, y, n);
    if(resp < n){
        printf("Error writing to output file %s\n", fo_name);
        free(x);
        free(y);
        return -1;
    }
    return 0;
}

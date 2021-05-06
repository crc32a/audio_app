#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("usage is %s <out file> ", prog);
    printf("<in file 1> <in file 2>\n");
    printf("\n");
    return 0;
}


int main(int argc, char **argv){
    cmp_t *in1;
    cmp_t *in2;
    cmp_t *out;
    cmp_t *tail;
    char *fo_name;
    char *f1_name;
    char *f2_name;
    int no;
    int n;
    int n1;
    int n2;
    int tsize;
    int status;
    int i;

    if(argc < 4){
        usage(argv[0]);
        return 0;
    }
    fo_name = argv[1];
    f1_name = argv[2];
    f2_name = argv[3];

    printf("input file 1: %s\n", f1_name);
    printf("input file 2: %s\n", f2_name);
    printf("output file: %s\n", fo_name);

    n1 = nsize(f1_name);
    if(n1 < 0) {
        printf("Error getting entry count for file %s\n", f1_name);
        return 0;
    }
    n2 = nsize(f2_name);
    if(n2 < 0){
        printf("Error getting entry count for file %s\n", f2_name);
        return 0;        
    }

    tsize = sizeof(cmp_t)*n1;
    in1 = (cmp_t *)malloc(tsize);
    if(in1 == NULL){
        printf("Error allocating %d bytes for input file ", tsize);
        printf("%s\n", f1_name);
        return -1;
    }
    tsize = sizeof(cmp_t)*n2;
    in2 = (cmp_t *)malloc(tsize);
    if(in2 == NULL){
        printf("Error allocating %d bytes for input file ", tsize);
        printf("%s\n", f2_name);
        free(in1);
        return -1;
    }
    if(n1 < n2){
        no = n2;
        n = n1;
        tail = in2;
    } else {
        no = n1;
        n = n2;
        tail = in1;
    }
    printf("input file1 %s has %d samples\n", f1_name, n1);
    printf("input file2 %s has %d samples\n", f2_name, n2);
    printf("output file %s has %d entries\n");
    tsize = sizeof(cmp_t)*no;
    out = (cmp_t *)malloc(tsize);
    if(out == NULL){
        printf("Error allocating %d bytes for output file ", tsize);
        printf("%s\n", fo_name);
        free(in1);
        free(in2);
        return -1;
    }
    printf("input file1 %s has %d samples\n", f1_name, n1);
    printf("input file2 %s has %d samples\n", f2_name, n2);
    printf("output file %s has %d entries\n", fo_name);

    printf("Reading file %s\n", f1_name);
    status = dft_fread(f1_name,in1,n1);
    if(status <0){
        printf("Error reading file %s\n", f1_name);
        free(in1);
        free(in2);
        free(out);
        return -1;
        
    }
    
    printf("Reading file %s\n", f2_name);
    status = dft_fread(f2_name,in2,n2);
    if(status <0){
        printf("Error reading file %s\n", f2_name);
        free(in1);
        free(in2);
        free(out);
        return -1;
        
    }
    
    printf("Mixing the signals\n");
    i = 0;
    while(i<n){
        out[i].r = in1[i].r + in2[i].r;
        out[i].i = in1[i].i + in2[i].i;
        i++;
    }
    while(i<no){
        out[i].r = tail[i].r;
        out[i].i = tail[i].i;
        i++;
    }
    free(in1);
    free(in2);
    printf("Writing to file %s\n", fo_name);
    status = dft_fwrite(fo_name, out, no);
    printf("status of write is %d\n", status);
    free(out);
    return 0;
}

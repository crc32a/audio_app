#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("usage is %s <n>\n", prog);
    printf("\n");
    printf("Compute the dft of n nputs\n");
    return 0;
}

int main(int argc, char **argv){
    cmp_t *in;
    cmp_t *out;
    int i;
    int n;
    int tmp_size;
    if(argc<2){
        usage(argv[0]);
        return 0;
    }
    n = atoi(argv[1]);
    tmp_size = sizeof(cmp_t) * n;
    in = (cmp_t *)malloc(tmp_size);
    if(in == NULL){
        printf("Error allocating %d bytes for in buffer\n");
        return -1;
    }
    out = (cmp_t *)malloc(tmp_size);
    if(out == NULL){
        printf("Error allocating %d bytes for out buffer\n");
        free(in);
        return -1;
    }
    for(i=0; i<n; i++){
        in[i].r = i;
        in[i].i = 0.0;
        out[i].r = 0.0;
        out[i].i = 0.0;
    }
    printf("input\n");
    fout_cmp(stdout, in, n);
    dft_cmp(out, in, n);
    printf("dft(input)\n");
    fout_cmp(stdout, out, n);
    idft_cmp(out, in, n);
    printf("idft(output)\n");
    fout_cmp(stdout, out, n);
    free(in);
    free(out);
    return 0;
}

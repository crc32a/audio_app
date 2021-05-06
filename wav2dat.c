#include<sndfile.h>
#include<stdlib.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("Usage is %s <file_out> <file_out>\n", prog);
    printf(":\n");
    printf("Read the input wav file and convert it to\n");
    printf("a dat file for processing\n");
    return 0;
}

int main(int argc, char **argv){
    char *fi_name;
    char *fo_name;
    cmp_t *dat;
    SNDFILE *fi;
    FILE *fo;
    SF_INFO si;
    int tsize;
    int sr;
    int resp;
    int n;

    

    if(argc < 3){
        usage(argv[0]);
        return 0;
    }
    fo_name = argv[1];
    fi_name = argv[2];

    fi = sf_open(fi_name, SFM_READ, &si);

    printf("Sound info for file %s\n", fi_name);
    printf("-------------------------------\n");
    printf("frame count:      %d\n", si.frames);
    printf("sample rate:      %d\n", si.samplerate);
    printf("channels count:   %d\n", si.channels);
    printf("format:           %x\n", si.format);
    printf("sections:         %d\n", si.sections);
    printf("seekable:         %d\n", si.seekable);
    sf_close(fi);
    sr = si.samplerate;
    n = si.frames;
    tsize = sizeof(cmp_t) * n;
    dat = (cmp_t *)malloc(tsize);
    if(dat == NULL){
        printf("Error allocating %d bytes for input\n", tsize);
        return -1;
    }
    resp = fread_wav(fi_name, &dat);
    if(resp < n){
        printf("Error reading file %s\n", fi_name);
        free(dat);
        return -1;
    }

    printf("read all wave data from %s\n", fi_name);
    printf("Writing to dat file %s\n", fo_name);
    resp = dft_fwrite(fo_name, dat, n);
    if(resp <n){
        printf("Error writing to %s\n", fo_name);
        perror("");
        free(dat);
        return -1;
    }
    return 0;
}


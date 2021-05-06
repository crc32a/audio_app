#include<sndfile.h>
#include<stdio.h>
#include<dft.h>

int usage(char *prog){
    printf("Usage is %s <file_name>\n", prog);
    printf("\n");
    printf("Read information about the audio file specified\n");
    printf("by the file name\n");
    return 0;
}

int main(int argc, char **argv){
    char *file_name;
    SNDFILE *fp;
    SF_INFO si;
    si.format = 0;

    if(argc<2){
        usage(argv[0]);
        return 0;
    }
    file_name = argv[1];
    printf("Opening file %s\n", file_name);
    fp = sf_open(file_name, SFM_READ, &si);
    if(fp == NULL){
        printf("Error opening file %sn", file_name);
        perror("");
        return 0;
    }
    printf("Sound info for file %s\n", file_name);
    printf("-------------------------------\n");
    printf("frame count:    %d\n", si.frames);
    printf("sample rate:    %d\n", si.samplerate);
    printf("channels count: %d\n", si.channels);
    printf("format:         %x\n", si.format);
    printf("sections:       %d\n", si.sections);
    printf("seekable:       %d\n", si.seekable);
    sf_close(fp);
    return 0;
}

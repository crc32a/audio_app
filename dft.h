#include<stdio.h>
#ifndef DFT_H
#define DFT_H

typedef struct {
    double r;
    double i;
}cmp_t;

typedef struct {
    int h;
    int size;
    int used;
    cmp_t *data;
}ring_t;

#endif

double arg_cmp(cmp_t *p);
double mod_cmp(cmp_t *p);
int add_cmp(cmp_t *a, cmp_t *p, cmp_t *q);
int sub_cmp(cmp_t *a, cmp_t *p, cmp_t *q);
int mul_cmp(cmp_t *a, cmp_t *p, cmp_t *q);
int div_cmp(cmp_t *a, cmp_t *p,  cmp_t  *q);
int exp_cmp(cmp_t *a, cmp_t *q);
int sqrt_cmp(cmp_t *a, cmp_t *q);
int dft_cmp(cmp_t *a, cmp_t *p, int n);
int idft_cmp(cmp_t *a, cmp_t *p, int n);
int fout_cmp(FILE *fp, cmp_t *p, int n);
int dft_fwrite(char *file_name, cmp_t *v, int n);
int dft_fread(char *file_name, cmp_t *v, int n);
int nsize(char *file_name);
int sigsquare(cmp_t *o, double a, double f, double ph,
int s, int n);
int sigsine(cmp_t *o, double a,double f,double ph,
int s, int n);
int sigsaw(cmp_t *o, double a,double f,double ph,
int s, int n);
int sigmix(cmp_t *out,cmp_t * in1, cmp_t *in2,double a,int n);
long fsize(FILE *file);
int nsize(char *file_name);
int sigscale(cmp_t *out, cmp_t *in, double scale, int n);
int fwrite_wav(char *file_name, cmp_t *p, int sr,int n);
int fread_wav(char *file_name, cmp_t **p);
double normalize_r(cmp_t *wav,int n);
double normalize_i(cmp_t *wav,int n);
double average(double *p, int n);
int e1filter(cmp_t *out, cmp_t *in, int n);
int e2filter(cmp_t *out, cmp_t *in, double a, int n);
int biquadcoeff(double **aa, double **bo,double sr, 
double f, double q);
int biquadfilter(cmp_t *y, cmp_t *x, double *a, double *b,
int nc, int n);

ring_t *ring_init(int size,int *err);
int ring_print(ring_t *r);
int ring_used(ring_t *r);
int ring_size(ring_t *r);
int ring_free(ring_t *r);
int ring_add(ring_t *r,cmp_t *data,int n);
int ring_get(ring_t *r,cmp_t *data,int n);
int ring_dec(ring_t *r,int n);
int ring_resize(ring_t *r,int n,int *err);

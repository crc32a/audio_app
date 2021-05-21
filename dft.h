#include<stdio.h>
#ifndef DFT_H
#define DFT_H

typedef struct {
    double r;
    double i;
} cmp_t;

#endif

double arg_cmp(cmp_t *p);
double mod_cmp(cmp_t *p);
int add_cmp(cmp_t *a, cmp_t *p, cmp_t *q);
int sub_cmp(cmp_t *a, cmp_t *p, cmp_t *q);
int mul_cmp(cmp_t *a, cmp_t *p, cmp_t *q);
int div_cmp(cmp_t *a, cmp_t *p, cmp_t *q);
int exp_cmp(cmp_t *a, cmp_t *q);
int sqrt_cmp(cmp_t *a, cmp_t *q);
int dft_cmp(cmp_t *a, cmp_t *p, int64_t n);
int idft_cmp(cmp_t *a, cmp_t *p, int64_t n);
int fout_cmp(FILE *fp, cmp_t *p, int64_t n);
int64_t dft_fwrite(char *file_name, cmp_t *v, int64_t n);
int64_t dft_fread(char *file_name, cmp_t *v, int64_t n);
int64_t nsize(char *file_name);
int sigsquare(cmp_t *o, double a, double f, double ph,
        int s, int64_t n);
int sigsine(cmp_t *o, double a, double f, double ph,
        int s, int64_t n);
int sigsaw(cmp_t *o, double a, double f, double ph,
        int s, int64_t n);
int sigmix(cmp_t *out, cmp_t * in1, cmp_t *in2, double a, int64_t n);
int64_t fsize(FILE *file);
int64_t nsize(char *file_name);
int sigscale(cmp_t *out, cmp_t *in, double scale, int64_t n);
int64_t fwrite_wav(char *file_name, cmp_t *p, int sr, int64_t n);
int64_t fread_wav(char *file_name, cmp_t **p);
double normalize_r(cmp_t *wav, int64_t n);
double normalize_i(cmp_t *wav, int64_t n);
double average(double *p, int64_t n);
int e1filter(cmp_t *out, cmp_t *in, int64_t n);
int e2filter(cmp_t *out, cmp_t *in, double a, int64_t n);
int biquadcoeff(double **aa, double **bo, double sr,
        double f, double q);
int biquadfilter(cmp_t *y, cmp_t *x, double *a, double *b,
        int nc, int64_t n);

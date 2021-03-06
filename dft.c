#include<sndfile.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<inttypes.h>
#include<math.h>
#include<dft.h>

#define NITEMS 128*1024

double mod_cmp(cmp_t *p) {
    return sqrt(p->r * p->r + p->i * p->i);
}

double arg_cmp(cmp_t *p) {
    return atan2(p->i, p->r);
}

int add_cmp(cmp_t *a, cmp_t *p, cmp_t *q) {
    a->r = p->r + q->r;
    a->i = p->i + q->i;
    return 0;
}

int sub_cmp(cmp_t *a, cmp_t *p, cmp_t *q) {
    a->r = p->r - q->r;
    a->i = p->i - q->i;
    return 0;
}

int mul_cmp(cmp_t *a, cmp_t *p, cmp_t *q) {
    a->r = p->r * q->r - p->i * q->i;
    a->i = p->r * q->i + p->i * q->r;
    return 0;
}

int div_cmp(cmp_t *a, cmp_t *p, cmp_t *q) {
    double aabb = 1.0 / (q->r * q->r + q->i * q->i);
    a->r = (p->r * q->r + p->i * q->i) * aabb;
    a->i = (q->r * p->i - p->r * q->i) * aabb;
    return 0;
}

int exp_cmp(cmp_t *a, cmp_t *q) {
    double ex = exp(q->r);
    double sy = sin(q->i);
    double cy = cos(q->i);
    a->r = ex*cy;
    a->i = ex*sy;
    return 0;
}

int sqrt_cmp(cmp_t *a, cmp_t *p) {
    double a2b2s = sqrt(p->r * p->r + p->i * p->i);
    a->r = sqrt(p->r + a2b2s * 0.5);
    double t = sqrt((-p->r + a2b2s)*0.5);
    if (p->i < 0.0) {
        a->i = 0 - t;
    } else {
        a->i = t;
    }
    return 0;
}

int dft_cmp(cmp_t *a, cmp_t *p, int64_t n) {
    int64_t k;
    int64_t t;
    double rsum;
    double isum;
    double thc = 2 * M_PI / (double) n;
    double th;
    double st;
    double ct;
    for (k = 0; k < n; k++) {
        rsum = 0.0;
        isum = 0.0;
        for (t = 0; t < n; t++) {
            th = thc * t*k;
            st = sin(th);
            ct = cos(th);
            rsum += p[t].r * ct + p[t].i*st;
            isum += p[t].i * ct - p[t].r*st;
        }
        a[k].r = rsum;
        a[k].i = isum;
    }
    return 0;
}

int idft_cmp(cmp_t *a, cmp_t *p, int64_t n) {
    int64_t k;
    int64_t t;
    double rsum;
    double isum;
    double thc = 2 * M_PI / (double) n;
    double th;
    double st;
    double ct;
    for (k = 0; k < n; k++) {
        rsum = 0.0;
        isum = 0.0;
        for (t = 0; t < n; t++) {
            th = thc * t*k;
            st = sin(th);
            ct = cos(th);
            rsum += p[t].r * ct - p[t].i*st;
            isum += p[t].r * ct + p[t].i*st;
        }
        a[k].r = rsum;
        a[k].i = isum;
    }
    return 0;
}

int fout_cmp(FILE *fp, cmp_t *p, int64_t n) {
    int64_t i;
    for (i = 0; i < n; i++) {
        fprintf(fp, "wav[%7d]=%f,%f\n", i, p[i].r, p[i].i);
    }
    return 0;
}

int sigsine(cmp_t *o, double a, double f, double ph,
        int s, int64_t n) {
    double cons;
    int64_t i;

    cons = 2 * M_PI * f / (double) s;
    for (i = 0; i < n; i++) {
        o[i].r = a * sin(cons * i + ph);
        o[i].i = 0.0;
    }
    return 0;
}

int sigsquare(cmp_t *o, double a, double f, double ph,
        int s, int64_t n) {
    double wl;
    double th;
    int64_t t;
    int64_t i;

    wl = f / s;
    th = s * ph / (M_PI * f);
    for (i = 0; i < n; i++) {
        t = (int) (wl * (i + th));
        o[i].r = a * (1.0 - (double) (t & 1) * 2.0);
        o[i].i = 0.0;
    }
    return 0;
}

int sigsaw(cmp_t *out, double a, double f, double ph,
        int s, int64_t n) {
    double wl;
    double m;
    double a2;
    double th;
    int64_t i;

    wl = s / f;
    a2 = 2.0 * a;
    m = a2 / wl;
    th = (wl * ph) / (2 * M_PI);

    for (i = 0; i < n; i++) {
        out[i].r = fmod(m * (i + th), a2) - a;
        out[i].i = 0.0;
    }
    return 0;
}

int sigmix(cmp_t *out, cmp_t * in1, cmp_t *in2, double a, int64_t n) {
    int64_t i;
    for (i = 0; i < n; i++) {
        out[i].r = a * (in1[i].r + in2[i].r);
        out[i].i = a * (in1[i].i + in2[i].i);
    }
    return 0;
}

int64_t dft_fwrite(char *file_name, cmp_t *v, int64_t n) {
    FILE *fp;
    int64_t i;
    int nwrite;
    int nwritten;
    int64_t ntotal;

    ntotal = 0;
    fp = fopen(file_name, "w");
    if (fp == NULL) return -1;
    i = 0;
    while (n > 0) {
        nwrite = NITEMS;
        if (n < nwrite) nwrite = n;
        nwritten = fwrite(v + i, sizeof (cmp_t), nwrite, fp);
        if (nwritten < 0) return -1;
        ntotal += nwritten;
        n -= nwritten;
        i += nwritten;
    }
    fclose(fp);
    return ntotal;
}

int64_t dft_fread(char *file_name, cmp_t *v, int64_t n) {
    FILE *fp;
    int64_t i;
    int nread;
    int nreaded;
    int nitems;
    int64_t ntotal;

    nitems = nsize(file_name);
    if (nitems < 0) {
        return -1;
    }
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        return -1;
    }
    ntotal = 0;
    if (nitems < n) {
        n = nitems;
    }
    i = 0;
    while (n > 0 && (!feof(fp) || !ferror(fp))) {
        nread = NITEMS;
        if (n < nread) nread = n;
        nreaded = fread(v + i, sizeof (cmp_t), nread, fp);
        if (nreaded < 0) return -1;
        ntotal += nreaded;
        n -= nreaded;
        i += nreaded;
    }
    fclose(fp);
    return ntotal;
}

int64_t nsize(char *file_name) {
    FILE *fp;
    int64_t bsize;
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        return -1;
    }
    bsize = fsize(fp);
    fclose(fp);
    return bsize / sizeof (cmp_t);
}

int64_t fsize(FILE *file) {
    int64_t old_pos;
    int64_t size;
    old_pos = ftell(file);
    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    fseek(file, old_pos, SEEK_SET);
    return (size);
}

int sigscale(cmp_t *out, cmp_t *in, double scale, int64_t n) {
    int64_t i;
    for (i = 0; i < n; i++) {
        out[i].r = scale * in[i].r;
        out[i].i = scale * in[i].i;
    }
    return 0;
}

int64_t fwrite_wav(char *file_name, cmp_t *in, int sr, int64_t n) {
    double *wav;
    SNDFILE *fp;
    SF_INFO sinfo;
    int64_t nwrite;
    int64_t nwritten;
    size_t tsize;
    int64_t i;

    memset(&sinfo, 0, sizeof (sinfo));
    sinfo.frames = n;
    sinfo.samplerate = sr;
    sinfo.channels = 1;
    sinfo.format = (SF_FORMAT_WAV | SF_FORMAT_PCM_16);
    fp = sf_open(file_name, SFM_WRITE, &sinfo);
    if (fp == NULL) {
        sf_perror(fp);
        return -1;
    }

    tsize = sizeof (double)*NITEMS;
    wav = (double *) malloc(tsize);
    if (wav == NULL) {
        sf_close(fp);
        return -1;
    }

    nwritten = 0;
    while (n > 0) {
        // Copy buffer
        nwrite = NITEMS;
        if (n < NITEMS) {
            nwrite = n;
        }
        for (i = 0; i < nwrite; i++) {
            wav[i] = in[nwritten + i].r;
        }

        // write buffer
        sf_writef_double(fp, wav, nwrite);

        n -= nwrite;
        nwritten += nwrite;
    }
    free(wav);
    sf_close(fp);
    return 0;
}

double normalize_r(cmp_t *wav, int64_t n) {
    int64_t i;
    double max;
    double f;
    double t;
    max = 0.0;
    for (i = 0; i < n; i++) {
        t = fabs(wav[i].r);
        if (max < t) max = t;
    }
    f = 1.0 / max;
    for (i = 0; i < n; i++) {
        wav[i].r *= f;
    }
    return (max);
}

double normalize_i(cmp_t *wav, int64_t n) {
    int64_t i;
    double max;
    double f;
    double t;
    max = 0.0;
    for (i = 0; i < n; i++) {
        t = fabs(wav[i].i);
        if (max < t) max = t;
    }
    f = 1.0 / max;
    for (i = 0; i < n; i++) {
        wav[i].i *= f;
    }
    return (max);
}

double average(double *p, int64_t n) {
    double sum = 0.0;
    int64_t i;

    for (i = 0; i < n; i++) {
        sum += p[i];
    }
    return sum / n;
}

double stddev(double *p, int64_t n) {
    double sum;
    double avg;
    double tmp;
    int64_t i;

    sum = 0.0;
    for (i = 0; i < n; i++) {
        sum += p[i];
    }
    avg = sum / n;
    sum = 0.0;
    for (i = 0; i < n; i++) {
        tmp = p[i] - avg;
        sum += tmp * tmp;
    }
    return sqrt(sum / n);
}

int e1filter(cmp_t *out, cmp_t *in, int64_t n) {
    int64_t i;

    out[0].r = 0.0;
    out[0].i = 0.0;
    for (i = 1; i < n; i++) {
        out[i].r = in[i].r - in[i - 1].r;
        out[i].i = in[i].i - in[i - 1].i;
    }
    return 0;
}

int e2filter(cmp_t *out, cmp_t *in, double a, int64_t n) {
    int64_t i;

    out[0].r = in[0].r;
    out[0].i = in[1].i;

    for (i = 1; i < n; i++) {
        out[i].r = a * out[i - 1].r + (1 - a) * in[i].r;
        out[i].i = a * out[i - 1].i + (1 - a) * in[i].i;
    }
    return 0;
}

int64_t fread_wav(char *file_name, cmp_t **p) {
    SNDFILE *fp;
    SF_INFO si;
    double *dbuff;
    cmp_t *out;
    size_t tsize;
    int64_t nread;
    int64_t n;
    int64_t i;
    int channels;

    *p = NULL;
    si.format = 0;
    fp = sf_open(file_name, SFM_READ, &si);
    if (fp == NULL) {
        return -1;
    }
    n = si.frames;
    channels = si.channels;

    tsize = sizeof (double)*channels*n;
    dbuff = (double *) malloc(tsize);
    if (dbuff == 0) {
        sf_close(fp);
        return -1;
    }
    tsize = sizeof (cmp_t) * n;
    out = (cmp_t *) malloc(tsize);
    if (out == NULL) {
        free(dbuff);
        sf_close(fp);
        return -1;
    }
    nread = sf_readf_double(fp, dbuff, n);
    if (nread < n) {
        free(dbuff);
        free(out);
        sf_close(fp);
        return -1;
    }

    for (i = 0; i < n; i++) {
        out[i].r = dbuff[i * channels];
        out[i].i = 0.0;
    }
    free(dbuff);
    *p = out;
    sf_close(fp);
    return n;
}

int biquadcoeff(double **ao, double **bo, double sr,
        double f, double q) {
    double *a;
    double *b;
    double k;
    double norm;
    double k2;
    size_t tsize;

    tsize = sizeof (double) * 3;
    a = (double *) malloc(tsize);
    if (a == NULL) {
        return -1;
    }
    b = (double *) malloc(tsize);
    if (b == NULL) {
        free(a);
        return -1;
    }
    *ao = a;
    *bo = b;
    k = tan(M_PI * f / sr);
    k2 = k * k;
    norm = 1.0 / (1.0 + k / q + k2);
    b[0] = k2 * norm;
    b[1] = 2.0 * b[0];
    b[2] = b[0];
    a[0] = 0.0;
    a[1] = 2 * norm * (k2 - 1.0);
    a[2] = norm * (1.0 - k / q + k2);
    return 3;
}

int biquadfilter(cmp_t *y, cmp_t *x, double *a, double *b,
        int nc, int64_t n) {
    double sum;
    int64_t i;
    int64_t j;


    for (i = 0; i < nc; i++) {
        y[i].r = 0.0;
        y[i].i = 0.0;
    }
    for (i = nc - 1; i < n; i++) {
        sum = 0.0;
        for (j = 0; j < nc; j++) {
            sum += x[i - j].r - y[i - j].r;
        }
        y[i].r = sum;
        y[i].i = 0.0;
    }
    return 0;
}

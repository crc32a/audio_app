#include<sndfile.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<math.h>
#include<dft.h>

#define NITEMS 128*1024

ring_t *ring_init(int size,int *err) {
    cmp_t *data;
    ring_t *r;
    data = (cmp_t *)malloc(sizeof(cmp_t)*size);
    if(data==NULL) {
        *err=1;
        return NULL;
    }
    r = (ring_t *)malloc(sizeof(ring_t));
    if(r==NULL) {
        free(data);
        *err=2;
        return NULL;
    }
    r->size = size;
    r->h = 0;
    r->used = 0;
    r->data = data;
    *err=0;
    return r;
}

int ring_used(ring_t *r) {
    return r->used;
}

int ring_size(ring_t *r) {
    return r->size;
}

int ring_free(ring_t *r) {
    return r->size - r->used;
}

int ring_print(ring_t *r) {
    int i;
    int j;
    int h = r->h;
    int s = r->size;
    int u = r->used;
    int f = r->size - r->used;

    printf("    ");
    for(i=0;i<s;i++) {
        if(i==h) {
            printf("%s","h");
        }
        else {
            printf("%s"," ");
        }
    }
    printf("\n    ");

    for(i=0;i<s;i++) {
        if(i%10==0) {
            printf("%i",i/10);
        }
        else {
            printf(" ");
        }
    }
    printf("\n    ");

    for(i=0;i<s;i++) {
        printf("%i",i%10);
    }
    printf("\n    ");

    for(i=0;i<s;i++) {
        j = (i + h)%s;
        printf("%6d: %f, %f\n",i,r->data[j].r,r->data[j].i);
    }
    printf("\n    ");
    printf("u=%i f=%i s=%i\n",u,f,s);
    return 0;
}

int ring_get(ring_t *r,cmp_t *data,int n) {
    int u = r->used;
    int di = 0;
    int ri = r->h;
    if(r->size == 0) return 0;
    while( di < n && u > 0) {
        data[di] = r->data[ri];
        ri = (ri+1)%r->size;
        di++;
        u--;
    }
    return di;
}

int ring_add(ring_t *r,cmp_t *data,int n) {
    int f  = ring_free(r);
    int di = 0; // index for parameter data
    int ri;
    if(r->size == 0) return 0;
    ri = (r->h+r->used)%r->size;
    while(di < n && f>0) {
        r->data[ri] = data[di];
        ri = (ri+1)%r->size;
        di++;
        f--;
    }
    r->used += di;
    return di;
}

int ring_dec(ring_t *r,int n) {
    n = (n < r->used) ? n : r->used;
    if(r->size == 0) return 0;
    r->h = (r->h + n)%r->size;
    r->used -= n;
    return n;
}

int ring_resize(ring_t *r,int new_size,int *err) {
    int ri;
    int i;
    int used;
    int size;
    cmp_t *ndata;
    cmp_t *data;
    used = r->used;
    data = r->data;
    size = r->size;
    if(new_size < used) {
        *err = 1;
        return 0;
    }
    ndata = (cmp_t *)malloc(sizeof(cmp_t)*new_size);
    if(ndata == NULL) {
        *err = 2;
        return 0;
    }

    if(size == 0) {
        r->h = 0;
        r->size = new_size;
        free(r->data);
        r->data = ndata;
        err = 0;
        return 1;
    }

    ri = r->h;
    for(i=0;i < used;i++) {
        ndata[i] = data[ri];
        ri = (ri+1)%size;
    }

    free(data);
    r->data = ndata;
    r->h = 0;
    r->size=new_size;
    *err=0;
    return 1;
}

double mod_cmp(cmp_t *p){
    return sqrt(p->r*p->r + p->i*p->i);
}

double arg_cmp(cmp_t *p){
    return atan2(p->i, p->r);
}

int add_cmp(cmp_t *a, cmp_t *p, cmp_t *q){
   a->r = p->r + q->r;
   a->i = p->i + q->i;
   return 0;
}

int sub_cmp(cmp_t *a, cmp_t *p, cmp_t *q){
   a->r = p->r - q->r;
   a->i = p->i - q->i;
   return 0;
}

int mul_cmp(cmp_t *a, cmp_t *p, cmp_t *q){
    a->r = p->r*q->r - p->i*q->i;
    a->i = p->r*q->i + p->i*q->r;
    return 0;
}

int div_cmp(cmp_t *a, cmp_t *p, cmp_t *q){
    double aabb = 1.0/(q->r*q->r + q->i*q->i);
    a->r = (p->r*q->r + p->i*q->i)*aabb;
    a->i = (q->r*p->i - p->r*q->i)*aabb;
    return 0;
}

int exp_cmp(cmp_t *a, cmp_t *q){
    double ex = exp(q->r);
    double sy = sin(q->i);
    double cy = cos(q->i);
    a->r = ex*cy;
    a->i = ex*sy;
    return 0;
}

int sqrt_cmp(cmp_t *a, cmp_t *p){
    double a2b2s = sqrt(p->r*p->r + p->i*p->i);
    a->r = sqrt(p->r + a2b2s*0.5);
    double t = sqrt((-p->r + a2b2s)*0.5);
    if(p->i < 0.0){
        a->i = 0 - t;
    } else {
        a->i = t;
    }
    return 0;
}

int dft_cmp(cmp_t *a, cmp_t *p, int n){
    int k;
    int t;
    double rsum;
    double isum;
    double thc = 2*M_PI/(double)n;
    double th;
    double st;
    double ct;
    for(k=0;k<n;k++){
        rsum = 0.0;
        isum = 0.0;
        for(t=0;t<n;t++){
          th = thc*t*k;
          st = sin(th);
          ct = cos(th);
          rsum += p[t].r*ct + p[t].i*st;
          isum += p[t].i*ct - p[t].r*st;
        }
        a[k].r = rsum;
        a[k].i = isum;
    }
    return 0;
}

int idft_cmp(cmp_t *a,  cmp_t *p, int n){
    int k;
    int t;
    double rsum;
    double isum;
    double thc = 2*M_PI/(double)n;
    double th;
    double st;
    double ct;
    for(k=0;k<n;k++){
        rsum = 0.0;
        isum = 0.0;
        for(t=0;t<n;t++){
          th = thc*t*k;
          st = sin(th);
          ct = cos(th);
          rsum += p[t].r*ct - p[t].i*st;
          isum += p[t].r*ct + p[t].i*st;
        }
        a[k].r = rsum;
        a[k].i = isum;
    }
    return 0;
}

int fout_cmp(FILE *fp,cmp_t *p, int n){
    int i;
    for(i=0;i<n;i++){
        fprintf(fp, "wav[%7d]=%f,%f\n", i, p[i].r,p[i].i);
    }
    return 0;
}

int print_sigargs(cmp_t *o,double a,double f,double p,
int s, int n,char *name){
    printf("Sigsign report generator \"%s\"\n", name);
    printf("----------------------------------------------\n");
    printf("data = %p\n", o);
    printf("amplitude   = %f\n", a);
    printf("frequency   = %f\n", f);
    printf("phase       = %f\n", p);
    printf("samplerate  = %d\n", s);
    printf("n           = %d\n", n);
    printf("\n");
    return 0;
}

int sigsine(cmp_t *o, double a, double f,double ph,
int s, int n){
    double cons;
    int i;

    print_sigargs(o,a,f,ph,s,n,"sine");

    cons = 2*M_PI*f / (double)s;
    for(i=0;i<n;i++) {
         o[i].r = a*sin(cons*i + ph);
         o[i].i = 0.0;
    }
    return 0;
}

int sigsquare(cmp_t *o, double a, double f, double ph,
int s, int n){
    double wl;
    double th;
    int t;
    int i;

    wl = f / s;
    th =  s * ph / (M_PI * f);
    for(i=0;i<n;i++){
        t = (int)(wl * (i+th));
        o[i].r =  a*(1.0 - (double)(t & 1) * 2.0);
        o[i].i = 0.0;
    }
    return 0;
}

int sigsaw(cmp_t *out, double a, double f, double ph,
int s, int n){
    double wl;
    double m;
    double a2;
    double th;
    int i;

    wl = s / f;
    a2 = 2.0 * a;
    m = a2 / wl;
    th = (wl * ph) / (2 * M_PI);
    
    for(i=0;i<n;i++){
        out[i].r = fmod(m*(i + th), a2) - a;
        out[i].i = 0.0;
    }
    return 0;
}

int sigmix(cmp_t *out,cmp_t * in1, cmp_t *in2,double a,int n){
    int i;
    for(i=0;i<n;i++){
        out[i].r = a*(in1[i].r+in2[i].r);
        out[i].i = a*(in1[i].i+in2[i].i);
    }
    return 0;
}

int dft_fwrite(char *file_name,cmp_t *v,int n){
    FILE *fp;
    int i;
    int nwrite;
    int nwritten;
    int ntotal;

    ntotal = 0;
    fp = fopen (file_name, "w");
    if(fp == NULL) return -1;
    i = 0;
    while(n>0){
        nwrite = NITEMS;
        if(n < nwrite) nwrite = n;
        nwritten = fwrite( v+i, sizeof(cmp_t), nwrite, fp);
        if(nwritten <0) return -1;
        ntotal += nwritten;
        n -= nwritten;
        i += nwritten;
    }
    fclose(fp);
    return ntotal;
}

int dft_fread(char *file_name, cmp_t *v,int n){
    FILE *fp;;
    int i;
    int nread;
    int nreaded;
    int nitems;
    int ntotal;

    nitems = nsize(file_name);
    if(nitems<0) {
        return -1;
    }
    fp = fopen(file_name, "r");
    if(fp == NULL){
        return -1;
    }
    ntotal = 0;
    if(nitems < n){
        n = nitems;
    }
    i = 0;
    while(n>0 && (!feof(fp) || !ferror(fp))){
        nread = NITEMS;
        if(n < nread) nread = n;
        nreaded = fread(v + i, sizeof(cmp_t), nread, fp);
        if(nreaded < 0) return -1;
        ntotal += nreaded;
        n -= nreaded;
        i += nreaded;
    }
    fclose(fp);
    return ntotal;
}

int nsize(char *file_name){
    FILE *fp;
    int bsize;
    fp = fopen(file_name, "r");
    if(fp == NULL){
        return -1;
    }
    bsize = fsize(fp);
    fclose(fp);
    return  bsize / sizeof(cmp_t);
}

long fsize(FILE *file)
   {
    long old_pos, size;
    old_pos=ftell(file);
    fseek(file,0L,SEEK_END);
    size=ftell(file);
    fseek(file,old_pos,SEEK_SET);
    return(size);
   }

int sigscale(cmp_t *out, cmp_t *in,double scale, int n){
    int i;
    for(i=0;i<n;i++){
        out[i].r = scale * in[i].r;
        out[i].i = scale * in[i].i;
    }
    return 0;
}

int fwrite_wav(char *file_name, cmp_t *in, int sr,int n){
    double *wav;
    SNDFILE *fp;
    SF_INFO sinfo;
    int nwrite;
    int nwritten;
    int tsize;
    int i;

    memset(&sinfo,0,sizeof(sinfo));
    sinfo.frames = n;
    sinfo.samplerate=sr;
    sinfo.channels=1;
    sinfo.format=(SF_FORMAT_WAV|SF_FORMAT_PCM_16);
    fp = sf_open(file_name, SFM_WRITE, &sinfo);
    if(fp == NULL){
        sf_perror(fp);
        return -1;
    }
    
    tsize = sizeof(double)*NITEMS;
    wav = (double *)malloc(tsize);
    if(wav == NULL){
        sf_close(fp);
        return -1;
    }

    nwritten = 0;
    while(n>0){
        // Copy buffer
        nwrite = NITEMS;
        if(n<NITEMS){
            nwrite = n;
        }
        for(i=0;i<nwrite;i++){
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

double normalize_r(cmp_t *wav,int n){
    int i;
    double max;
    double f;
    double t;
    max=0.0;
    for(i=0;i<n;i++){
        t = fabs(wav[i].r);
        if(max<t) max=t;
    }
    f=1.0/max;
    for(i=0;i<n;i++) {
        wav[i].r*=f;
    }
    return(max);
}

double normalize_i(cmp_t *wav,int n){
    int i;
    double max;
    double f;
    double t;
    max=0.0;
    for(i=0;i<n;i++){
        t = fabs(wav[i].i);
        if(max<t) max=t;
    }
    f=1.0/max;
    for(i=0;i<n;i++) {
        wav[i].i*=f;
    }
    return(max);
}

double average(double *p, int n){
    double sum = 0.0;
    int i;

    for(i=0;i<n;i++){
        sum += p[i];
    }
    return sum / n;
}

double stddev(double *p, int n){
    double sum;
    double avg;
    double tmp;
    int i;

    sum = 0.0;
    for(i=0;i<n;i++){
        sum += p[i];
    }
    avg = sum / n;
    sum = 0.0;
    for(i=0;i<n;i++){
        tmp = p[i] - avg;
        sum += tmp * tmp;
    }
    return sqrt(sum / n);
}

int e1filter(cmp_t *out, cmp_t *in, int n){
    int i;

    out[0].r = 0.0;
    out[0].i = 0.0;
    for(i=1;i<n;i++){
        out[i].r = in[i].r - in[i-1].r;
        out[i].i = in[i].i - in[i-1].i;
    }
    return 0;
}

int e2filter(cmp_t *out, cmp_t *in, double a, int n){
    int i;

    out[0].r = in[0].r;
    out[0].i = in[1].i;

    for(i=1;i<n;i++){
        out[i].r = a*out[i-1].r + (1 - a)*in[i].r;
        out[i].i = a*out[i-1].i + (1 - a)*in[i].i;
    }
    return 0;
}

int fread_wav(char *file_name, cmp_t **p){
    SNDFILE *fp;
    SF_INFO si;
    double *dbuff;
    cmp_t *out;
    int tsize;
    int channels;
    int nread;
    int n;
    int i;

    *p = NULL;
    si.format = 0;
    fp = sf_open(file_name, SFM_READ, &si);
    if(fp == NULL){
        return -1;
    }
    n = si.frames;
    channels = si.channels;

    tsize = sizeof(double)*channels*n;
    dbuff = (double *)malloc(tsize);
    if(dbuff == 0){
        sf_close(fp);
        return -1;
    }
    tsize = sizeof(cmp_t)*n;
    out = (cmp_t *)malloc(tsize);
    if(out == NULL){
        free(dbuff);
        sf_close(fp);
        return -1;
    }
    nread = sf_readf_double(fp, dbuff, n);
    if(nread < n) {
        free(dbuff);
        free(out);
        sf_close(fp);
        return -1;
    }

    for(i=0;i<n;i++){
        out[i].r = dbuff[i*channels];
        out[i].i = 0.0;
    }
    free(dbuff);
    *p = out;
    sf_close(fp);
    return n;
}

int biquadcoeff(double **ao, double **bo,double sr, 
double f, double q){
    double *a;
    double *b;
    double k;
    double norm;
    double k2;
    int tsize;

    tsize = sizeof(double) * 3;
    a = (double *)malloc(tsize);
    if(a==NULL){
        return -1;
    }
    b = (double *)malloc(tsize);
    if(b == NULL){
        free(a);
        return -1;
    }
    *ao = a;
    *bo = b;
    k = tan(M_PI *f / sr);
    k2 = k * k;
    norm = 1.0 / (1.0 + k/q + k2);
    b[0] = k2 * norm;
    b[1] = 2.0 * b[0];
    b[2] = b[0];
    a[0] = 0.0;
    a[1] = 2 * norm * (k2 - 1.0);
    a[2] = norm * (1.0 - k/q + k2);
    return 3;
}

int biquadfilter(cmp_t *y, cmp_t *x, double *a, double *b, 
int nc, int n){
    double sum;
    int i;
    int j;
    

    for(i=0;i<nc;i++){
        y[i].r = 0.0;
        y[i].i = 0.0;
    }
    for(i=nc-1;i<n;i++){
        sum = 0.0;
        for(j=0;j<nc;j++){
            sum += x[i-j].r - y[i-j].r;
        }
        y[i].r = sum;
        y[i].i = 0.0;
    }
    return 0;
}

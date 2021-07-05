#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "threadpool.h"

#define PRECISION 1000 /* upper bound in BPP sum */

typedef struct {
    int arg;
    double res;
} bpp_t;

static void simple_func(void *arg)
{
    int n = *(int *) arg;

    //printf("processing : %d\n", n);
}

/* Use Bailey–Borwein–Plouffe formula to approximate PI */
static void bpp_func(void *arg)
{
    bpp_t *bpp = (bpp_t *) arg;
    int k = bpp->arg;
    double sum = (4.0 / (8 * k + 1)) - (2.0 / (8 * k + 4)) -
                 (1.0 / (8 * k + 5)) - (1.0 / (8 * k + 6));
    bpp->res = 1 / pow(16, k) * sum;
}

int main (int argc, char **argv) {
    struct timespec tt1, tt2;
    clock_gettime(CLOCK_REALTIME, &tt1);

    int nThreads = argc > 1 ? abs(atoi(argv[1])) : sysconf(_SC_NPROCESSORS_ONLN);
    int qSize = 0x100000;

    printf("creating pool with %d threads and queue size %d...\n", nThreads, qSize);

    // queue size must be 2^n
    threadpool_t *pool = threadpool_create(nThreads, qSize);

    printf("pool created...\n");

    /* bpp_t bpps[PRECISION + 1]; */
    /* for (int i = 0; i <= PRECISION; i++) { */
    /*     bpps[i].arg = i; */
    /*     bpps[i].res = 0; */
    /*     threadpool_push(pool, &bpp_func, (void *) &bpps[i]); */
    /* } */

    int sps[qSize + 1];
    for (int i = 0; i <= qSize; i++) {
        sps[i] = i;
        threadpool_push(pool, &simple_func, (void *) &sps[i]);
    }

    printf("tasks pushed...\n");

    threadpool_wait(pool);

    clock_gettime(CLOCK_REALTIME, &tt2);
    printf("tp consumes %lu nanoseconds!\n", tt2.tv_nsec - tt1.tv_nsec);

    /* double sum = 0; */
    /* for (int i = 0; i <= PRECISION; i++) { */
    /*     sum += bpps[i].res; */
    /* } */
    /* printf("calculated sum: %lf\n", sum); */
    return 0;
}

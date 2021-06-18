#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include "threadpool.h"

#define PRECISION 100 /* upper bound in BPP sum */

typedef struct {
    int arg;
    double res;
} bpp_t;

/* Use Bailey–Borwein–Plouffe formula to approximate PI */
static void bpp_func(void *arg)
{
    bpp_t *bpp = (bpp_t *) arg;
    int k = bpp->arg;
    double sum = (4.0 / (8 * k + 1)) - (2.0 / (8 * k + 4)) -
                 (1.0 / (8 * k + 5)) - (1.0 / (8 * k + 6));
    bpp->res = 1 / pow(16, k) * sum;
}

int main () {
    struct timespec tt1, tt2;
    clock_gettime(CLOCK_REALTIME, &tt1);

    threadpool_t *pool = threadpool_create(0x04, 0xFF);

    printf("pool created...\n");

    bpp_t bpps[PRECISION + 1];
    for (int i = 0; i <= PRECISION; i++) {
        bpps[i].arg = i;
        bpps[i].res = 0;
        threadpool_push(pool, &bpp_func, (void *) &bpps[i]);
    }

    printf("tasks pushed...\n");

    threadpool_wait(pool);

    clock_gettime(CLOCK_REALTIME, &tt2);
    printf("tp consumes %ld nanoseconds!\n", tt2.tv_nsec - tt1.tv_nsec);

    double sum = 0;
    for (int i = 0; i <= PRECISION; i++) {
        sum += bpps[i].res;
    }
    printf("calculated sum: %lf\n", sum);
    return 0;
}

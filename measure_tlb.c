#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include "utils.h"

static int sum = 0;

static inline void access_memory(char* ptr) {
    sum += *ptr;
}

void measure(int num_pages) {
    int stride = 731734319;

    struct timespec start;
    struct timespec now;
    uint64_t elapsed;


    uint64_t accesses = 0;
    uint64_t sum = 0; // don't optimize away our access.

    get_time(&start);
    while(1) {
        for (int i = 0; i < 10000; i++) {
            uint64_t hi = 64;
            uint64_t lo = 0;;
            for(int j=0; j<num_pages; j++) {
                uint64_t addr = hi << 39 | lo < 30;
                int *ptr = (int *) addr;
                sum = sum + *ptr;
                accesses++;
                if (++hi < 192) continue;
                hi = 64;
                if (lo++ < 256) continue;
                lo = 0;
            }
        }

        get_time(&now);
        elapsed = elapsed_nanos(&start, &now);
        if (elapsed > 1 * ONE_BILLION) break;
    }

 //   printf("accesses: %llu %llu %llu\n", accesses, elapsed, sum);

    double perSecond = accesses;
    perSecond = perSecond / elapsed;
    perSecond *= ONE_BILLION;

    printf("%d,%f\n", num_pages, perSecond);

}

int main(int argc, char** argv) {

    int num_pages = 10000;
    int page_size = 4096;
    int table_size = 1000;
    int count = 1000000;


    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 512; k++) {
                uint64_t addr = i;
                addr = addr << 5 | j;
                addr = addr << 9 | k;
                addr = addr << 30;
//                printf("%p\n", addr);
                void *p2 = mmap(addr, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);
                if (p2 == NULL) exit_with_error("shm_attach");
            }
        }
    }
    measure(65535);

    for (int i = 1; i < 65; i++) {
        measure(i * 1000);
    }

    for (int i = 1; i < 2048; i++) {
        measure(i);
    }
}
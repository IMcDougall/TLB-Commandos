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
    int stride = 113;

    struct timespec start;
    struct timespec now;
    uint64_t elapsed;


    uint64_t page = 0;
    uint64_t base = 2L << 44;

    uint64_t accesses = 0;
    int sum = 0; // don't optimize away our access.

    get_time(&start);
    while(1) {
        for (int i = 0; i < 1000000; i++) {
            page = (page + stride) % num_pages;
            uint64_t addr = base | page << 23;
//            fprintf(stderr, "Addr: %p\n", (int*)addr);
            sum = sum + ((int*)addr);
            accesses++;
        }

        get_time(&now);
        elapsed = elapsed_nanos(&start, &now);
        if (elapsed > 2 * ONE_BILLION) break;
    }

 //   printf("accesses: %llu %llu %llu\n", accesses, elapsed, sum);

    double perSecond = accesses;
    perSecond = perSecond / elapsed;
    perSecond *= ONE_BILLION;

    printf("num_pages:%d accesses:%llu a/s:%.0f sum:%d\n", num_pages, accesses, perSecond, sum);

}

int main(int argc, char** argv) {

    int num_pages = 10000;
    if(argc > 1) num_pages = atoi(argv[1]);
    int page_size = 4096;
    int table_size = 1000;
    int count = 1000000;


    uint64_t addr = 2L << 44;
    for(int i=0 ; i<num_pages; i++) {
//        printf("makeAddr: %p\n", (int*)addr);
        void *p2 = mmap(addr, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);
        if (p2 == NULL) exit_with_error("mmap");
        addr += 1 << 23;
    }

    measure(num_pages);
}
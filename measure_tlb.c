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

void measure(int num_pages, long sample_time) {
    int stride = 346051; //prime

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
            sum = sum + *((int*)addr);
            accesses++;
        }

        get_time(&now);
        elapsed = elapsed_nanos(&start, &now);
        if (elapsed > sample_time * ONE_BILLION) break;
    }

 //   printf("accesses: %llu %llu %llu\n", accesses, elapsed, sum);

    double perSecond = accesses;
    perSecond = perSecond / elapsed;
    perSecond *= ONE_BILLION;

    printf("num_pages:%d accesses:%llu a/s:%.0f sum:%d\n", num_pages, accesses, perSecond, sum);

}

int main(int argc, char** argv) {

    uint64_t  start_tlb = get_tlb_count();
    print_heap_stack_address();

    int num_pages = 10000;
    if(argc > 1) num_pages = atoi(argv[1]);

    int sample_time = 10;
    if(argc > 2) sample_time = atoi(argv[2]);
    int page_size = 4096;
    int table_size = 1000;
    int count = 1000000;

    uint64_t addr = 2L << 44;
    for(int i=0 ; i<num_pages; i++) {
        void *p2 = mmap((void*) addr, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);
        if (p2 == NULL) exit_with_error("mmap");
        if((uint64_t)p2 != addr) {
            printf("i=%d  addr:%016llx\n", i, p2);
            exit_with_error("address mismatch");
        }
        *((int*)addr) = i;
        addr += 1 << 23;
    }

    measure(num_pages, sample_time);
    uint64_t  end_tlb = get_tlb_count();
    uint64_t diff = end_tlb - start_tlb;
    printf("start: %lu end: %lu diff: %lu\n", start_tlb, end_tlb, diff);
}
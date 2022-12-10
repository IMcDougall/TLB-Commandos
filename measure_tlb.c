#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include "utils.h"

void measure(int num_pages, long sample_time) {
    int stride = find_next_prime((num_pages * 3) / 2);

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

    printf("num_pages:%d stride:%d accesses:%llu a/s:%.0f(%.2f) sum:%d\n", num_pages, stride, accesses, perSecond, perSecond/1000000.0, sum);

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
        uint64_t  start = get_tlb_count();
        void *p2 = mmap((void*) addr, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);
        uint64_t  alloc = get_tlb_count();
        if (p2 == -1) exit_with_error("mmap");
        *((int*)addr) = i; // put a value into the allocated buffer
        uint64_t  access = get_tlb_count();
        addr += 1 << 23;

        printf("start:%d alloc:%d access:%d\n", start, alloc, access);
    }

    for(int i=180; i<2560; i+=10) {
        measure(i, 1);
    }


    uint64_t  end_tlb = get_tlb_count();
    uint64_t diff = end_tlb - start_tlb;
    printf("start: %lu end: %lu diff: %lu\n", start_tlb, end_tlb, diff);
}
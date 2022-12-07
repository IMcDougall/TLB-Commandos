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


int cmpfunc (const void * a, const void * b) {
    return ( *(uint64_t *)a - *(uint64_t*)b );
}

void print_percentile(int num_measurements, uint64_t* measurements, float percentile) {
    float findex = percentile * num_measurements / 100.0;
    int index = (int) findex;
    if(index>=num_measurements) index = num_measurements-1;

    printf("%f (%d of %d) %lu\n", percentile, index, num_measurements, measurements[index]);
}


void measure(int num_pages, int num_measurements, int sample_count) {
    int stride = 346051; //prime

    struct timespec start;
    struct timespec now;
    uint64_t elapsed;


    uint64_t page = 0;
    uint64_t base = 2L << 44;

    uint64_t accesses = 0;
    int sum = 0; // don't optimize away our access.

    uint64_t measurements[num_measurements];

    get_time(&start);
    for(int j=0; j<num_measurements; j++) {
        get_time(&start);
        for (int i = 0; i < sample_count; i++) {
            page = (page + stride) % num_pages;
            uint64_t addr = base | page << 23;
            sum = sum + *((int*)addr);
            accesses++;
        }
        get_time(&now);
        measurements[j] = elapsed_nanos(&start, &now);
    }


    qsort(measurements, num_measurements, sizeof(uint64_t), cmpfunc);
    print_percentile(num_measurements, measurements, 0.0);
    print_percentile(num_measurements, measurements, 10.0);
    print_percentile(num_measurements, measurements, 50.0);
    print_percentile(num_measurements, measurements, 90.0);
    print_percentile(num_measurements, measurements, 99.0);
    print_percentile(num_measurements, measurements, 99.9);
    print_percentile(num_measurements, measurements, 99.99);
    print_percentile(num_measurements, measurements, 100);

    //   printf("accesses: %llu %llu %llu\n", accesses, elapsed, sum);

    double perSecond = accesses;
    perSecond = perSecond / elapsed;
    perSecond *= ONE_BILLION;

    printf("num_pages:%d accesses:%llu a/s:%.0f sum:%d\n", num_pages, accesses, perSecond, sum);

}

int main(int argc, char** argv) {

    uint64_t  start_tlb = get_tlb_count();

    int num_pages = 10000;
    if(argc > 1) num_pages = atoi(argv[1]);

    int num_measurements = 10000;
    if(argc > 2) num_measurements = atoi(argv[2]);

    int sample_size = 1000;
    if(argc > 2) sample_size = atoi(argv[3]);


    int page_size = 4096;
    int table_size = 1000;
    int count = 1000000;

    void* heap = malloc(1);
    if(heap == NULL) exit_with_error("malloc");
    uint64_t heap_addr = (uint64_t) heap;
    uint64_t stack_addr = (uint64_t) (&heap);
    printf("heap_addr:%llu(%lx)  stack_addr:%llu(%lx)\n", heap_addr, heap_addr, stack_addr, stack_addr);

    uint64_t addr = 2L << 44;
    for(int i=0 ; i<num_pages; i++) {
        void *p2 = mmap((void*) addr, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);
        if (p2 == NULL) exit_with_error("mmap");
        if((uint64_t)p2 != addr) exit_with_error("address mismatch");
        *((int*)addr) = i;
        addr += 1 << 23;
    }

    measure(num_pages, num_measurements, sample_size);
    uint64_t  end_tlb = get_tlb_count();
    uint64_t diff = end_tlb - start_tlb;
    printf("start: %lu end: %lu diff: %lu\n", start_tlb, end_tlb, diff);
}
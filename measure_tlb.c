#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>

#include "utils.h"

int is_verbose = 0;

void measure(int num_pages, long sample_time, int method) {
    int stride = find_next_prime((num_pages * 3) / 2);

    srand(time(NULL));   // Initialization, should only be called once.

    struct timespec start;
    struct timespec now;
    uint64_t elapsed;


    uint64_t page = 0;
    uint64_t base = 2L << 44;

    uint64_t accesses = 0;
    int sum = 0; // don't optimize away our access.

    get_time(&start);

    while (1) {
        switch(method) {
            case 1:
                for (int i = 0; i < 1000000; i++) {
                    page = rand() % num_pages;
                    uint64_t addr = base | page << 23;
                    sum = sum + *((int *) addr);
                    accesses++;
                }
                break;
            case 2:
                for (int i = 0; i < 1000000; i++) {
                    if(page++ > num_pages) page = 0;
                    uint64_t addr = base | page << 23;
                    sum = sum + *((int *) addr);
                    accesses++;
                }
                break;
            case 3:
                for (int i = 0; i < 1000000; i++) {
                    page = (page + stride) % num_pages;
                    uint64_t addr = base | page << 23;
                    sum = sum + *((int *) addr);
                    accesses++;
                }
                break;
            default:
                printf("unknown method:%d\n", method);
                exit(1);
        }

        get_time(&now);
        elapsed = elapsed_nanos(&start, &now);
        if (elapsed > sample_time * ONE_BILLION) break;
    }

    double perSecond = accesses;
    perSecond = perSecond / elapsed;
    perSecond *= ONE_BILLION;

    if (is_verbose) {
        printf("num_pages:%d stride:%d accesses:%llu a/s:%.0f(%.2f) sum:%d\n", num_pages, stride, accesses, perSecond,
               perSecond / 1000000.0, sum);
    } else {
        printf("%d %.2f\n", num_pages, perSecond/1000000.0);
    }
}

void usage() {
    printf("should add usage here\n");
    exit(1);
}

int main(int argc, char** argv) {

    int num_pages = 10000;
    int begin = 1;
    int end = 100;
    int increment = 1;
    int timeout = 1;
    int method = 2;

    int c;
    while ((c = getopt(argc, argv, "vp:b:e:i:t:m:")) != -1) {
        switch (c) {
            case 'p':
                num_pages = atoi(optarg);
                break;
            case 'b':
                begin = atoi(optarg);
                break;
            case 'e':
                end = atoi(optarg);
                break;
            case 'i':
                increment = atoi(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            case 'm':
                method = atoi(optarg);
                break;
            case 'v':
                is_verbose = 1;
                break;
             default:
                usage();
        }
    }

    char hostbuffer[256];
    int result = gethostname(hostbuffer, sizeof(hostbuffer));

    printf("%% pages: %d\n", num_pages);
    printf("%% begin: %d\n", begin);
    printf("%% end: %d\n", end);
    printf("%% increment: %d\n", increment);
    printf("%% timeout: %d\n", timeout);
    printf("%% method: %d\n", method);
    printf("%% hostname: %s\n", hostbuffer);


    uint64_t  start_tlb = get_tlb_count();
    if(is_verbose) print_heap_stack_address();

    uint64_t addr = 2L << 44;
    for(int i=0 ; i<num_pages; i++) {
        uint64_t  start = get_tlb_count();
        void *p2 = mmap((void*) addr, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);
        uint64_t  alloc = get_tlb_count();
        if (p2 == -1) exit_with_error("mmap");
        *((int*)addr) = i; // put a value into the allocated buffer
        uint64_t  access = get_tlb_count();
        addr += 1 << 23;

        if(is_verbose) printf("page:%d start:%d alloc:%d access:%d\n", i, start, alloc, access);
    }

    if(!is_verbose) printf("# numberOfPages accessesPerSecond\n");
    for(int i=begin; i<end; i+=increment) {
        measure(i, timeout, method);
    }


    uint64_t  end_tlb = get_tlb_count();
    uint64_t diff = end_tlb - start_tlb;
    if(is_verbose) printf("start: %lu end: %lu diff: %lu\n", start_tlb, end_tlb, diff);
}
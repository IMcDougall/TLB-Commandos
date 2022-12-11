#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

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

    printf("%f (%d of %d) %llu\n", percentile, index, num_measurements, measurements[index]);
}

uint64_t get_percentile(int num_measurements, uint64_t* measurements, float percentile) {
    float findex = percentile * num_measurements / 100.0;
    int index = (int) findex;
    if(index>=num_measurements) index = num_measurements-1;

    return measurements[index];
}


void measure(int num_pages, int num_measurements, int sample_count) {
    int stride = find_next_prime((num_pages * 3) / 2);

    struct timespec start;
    struct timespec now;
    uint64_t elapsed;


    uint64_t page = 0;
    uint64_t base = 2L << 44;

    uint64_t accesses = 0;
    int sum = 0; // don't optimize away our access.

    uint64_t measurements[num_measurements];

    get_time(&start);
    for (int j = 0; j < num_measurements; j++) {
        get_time(&start);
        for (int i = 0; i < sample_count; i++) {
            page = (page + stride) % num_pages;
            uint64_t addr = base | page << 23;
            sum = sum + *((int *) addr);
            accesses++;
        }
        get_time(&now);
        measurements[j] = elapsed_nanos(&start, &now);
    }


    qsort(measurements, num_measurements, sizeof(uint64_t), cmpfunc);
//    print_percentile(num_measurements, measurements, 0.0);
//    print_percentile(num_measurements, measurements, 10.0);
//    print_percentile(num_measurements, measurements, 50.0);
//    print_percentile(num_measurements, measurements, 90.0);
//    print_percentile(num_measurements, measurements, 99.0);
//    print_percentile(num_measurements, measurements, 99.9);
//    print_percentile(num_measurements, measurements, 99.99);
//    print_percentile(num_measurements, measurements, 100);

    double total = 0.0;
    for (int i = 0; i < num_measurements; i++) total += measurements[i];
    double average = total / (double) num_measurements;

    double error_squared = 0.0;
    for (int i = 0; i < num_measurements; i++) {
        double delta = average - (double) num_measurements;
        error_squared += delta * delta;
    }
    double variance = error_squared / (num_measurements - 1);
//    double stddev = sqrt(variance);

    printf("%d %llu %llu %llu %llu %llu %llu %llu %llu %.2f %.2f \n", num_pages,
           get_percentile(num_measurements, measurements, 0.0),
           get_percentile(num_measurements, measurements, 10.0),
           get_percentile(num_measurements, measurements, 50.0),
           get_percentile(num_measurements, measurements, 90.0),
           get_percentile(num_measurements, measurements, 99.0),
           get_percentile(num_measurements, measurements, 99.9),
           get_percentile(num_measurements, measurements, 99.99),
           get_percentile(num_measurements, measurements, 100),
           average, variance);
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
    int samples = 100000;
    int method = 2;
    int count = 1000;

    int c;
    while ((c = getopt(argc, argv, "vp:b:e:i:s:m:c:")) != -1) {
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
            case 's':
                samples = atoi(optarg);
                break;
            case 'c':
                count = atoi(optarg);
                break;
            case 'm':
                method = atoi(optarg);
                break;
             default:
                usage();
        }
    }

    char hostbuffer[256];
    int result = gethostname(hostbuffer, sizeof(hostbuffer));

    printf("%% hostname: %s\n", hostbuffer);

    printf("%% pages: %d\n", num_pages);
    printf("%% begin: %d\n", begin);
    printf("%% end: %d\n", end);
    printf("%% increment: %d\n", increment);
    printf("%% samples: %d\n", samples);
    printf("%% count: %d\n", count);
    printf("%% method: %d\n", method);


    uint64_t  start_tlb = get_tlb_count();
    print_heap_stack_address();

    uint64_t addr = 2L << 44;
    for(int i=0 ; i<num_pages; i++) {
        uint64_t  start = get_tlb_count();
        void *p2 = mmap((void*) addr, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);
        uint64_t  alloc = get_tlb_count();
        if ((int64_t)p2 == -1L) exit_with_error("mmap");
        *((int*)addr) = i; // put a value into the allocated buffer
        uint64_t  access = get_tlb_count();
        addr += 1 << 23;
    }

    printf("# pages p0 p10 p50 p90 p99 p99.9 p99.99 p100 average variance\n");

    for(int i=begin; i<end; i+=increment) {
        measure(i, samples, count);
    }
}
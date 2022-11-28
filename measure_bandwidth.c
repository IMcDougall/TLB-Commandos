//
// Created by James Sorenson on 11/28/22.
//
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

void measure(int buffer_size, u_int8_t* buffer) {
    int stride = 731319;

    struct timespec start;
    struct timespec now;
    uint64_t elapsed;

    uint64_t accesses = 0;
    uint64_t sum = 0; // don't optimize away our access.
    int index = 0;

    get_time(&start);
    while(1) {
        for (int i = 0; i < 1000000; i++) {
            sum += buffer[index];
            index = (index + stride) & buffer_size;
            accesses++;
        }

        get_time(&now);
        elapsed = elapsed_nanos(&start, &now);
        if (elapsed > 5 * ONE_BILLION) break;
    }

    //   printf("accesses: %llu %llu %llu\n", accesses, elapsed, sum);

    double perSecond = accesses;
    perSecond = perSecond / elapsed;
    perSecond *= ONE_BILLION;

    printf("%d %f %llu\n", buffer_size, perSecond/1000000.0, sum);

}

int main(int argc, char** argv) {

    int buffer_size = 32*1024*1024;
    if(argc>1) buffer_size = 1 << atoi(argv[1]);

    unsigned char* buffer = malloc(buffer_size);
    if(buffer == NULL) exit_with_error("malloc");

    measure(buffer_size, buffer);
}
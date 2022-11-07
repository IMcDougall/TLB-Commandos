#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/shm.h>

#include "utils.h"

static int sum = 0;

static inline void access_memory(char* ptr) {
    sum += *ptr;
}

int main(int argc, char** argv) {

    int num_pages = 10000;
    int page_size = 4096;
    int table_size = 1000;
    int count = 1000000;
    int stride = 731734319;

    unsigned int next = 0;

    int segment_id = shmget(IPC_PRIVATE, page_size, IPC_CREAT | 0600);
    if (segment_id == -1) exit_with_error("shmget");

    for(int i=2; i<6; i++) {
        for(int j=0; j<32; j++) {
            for(int k=0; k<512; k++) {
                uint64_t addr = i;
                addr = addr << 5 | j;
                addr = addr << 9 | k;
                addr = addr << 30;
                void *p2 = shmat(segment_id, (void *) addr, SHM_RND);
                if (p2 == NULL) exit_with_error("shm_attach");
            }
        }
    }

    int c;
    while ((c = getopt(argc, argv, "p:s:t:c:")) != -1) {
        switch (c) {
            case 'p':
                num_pages = atoi(optarg);;
                break;
            case 's':
                page_size = atoi(optarg);;
                break;
            case 't':
                table_size = atoi(optarg);;
                break;
            case 'c':
                count = atoi(optarg);;
                break;

            default:
                printf("Hello, World!\n");
                printf("do I have github hooked up correctly?\n");
        }
    }

    printf("num_pages: %d\n", num_pages);
    printf("page_size: %d\n", page_size);
    printf("stride: %d\n", stride);

    char** pages = (char**) malloc(num_pages * sizeof(char*));
    for(int i=0; i<num_pages ; i++) {
        pages[i] = malloc(page_size);
    }

    printf("Allocated!\n");

    struct timespec start;
    struct timespec now;
    uint64_t elapsed;

    uint64_t accesses = 0;

    get_time(&start);
    while(1) {
        for (int i = 0; i < count; i++) {
            access_memory(pages[next]);
            accesses++;
            next = (next + stride) % num_pages;
        }

        get_time(&now);
        elapsed = elapsed_nanos(&start, &now);
        if (elapsed > 3L * ONE_BILLION) break;

    }

    printf("accesses: %llu %llu\n", accesses, elapsed);

    double perSecond = accesses;
    perSecond = perSecond / elapsed;
    perSecond *= ONE_BILLION;

    printf("access per second:%f\n", perSecond);

    return 0;
}

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>



static void initialize_counters() {
//__asm__ __volatile__ ("mcr p15, 0, %0, c9, c12, 2" :: "r"(1<<31)); /* stop the cc */
//__asm__ __volatile__ ("mcr p15, 0, %0, c9, c12, 0" :: "r"(5));     /* initialize */
//__asm__ __volatile__ ("mcr p15, 0, %0, c9, c12, 1" :: "r"(1<<31)); /* start the cc */
}

static inline unsigned read_counter() {
    uint64_t tsc;
#ifdef __arm64
    __asm__ __volatile__ ("dmb sy");
    __asm__ __volatile__ ("mrs %0, cntvct_el0" : "=r"(tsc));
    __asm__ __volatile__ ("dmb sy");
#else
    unsigned a, d;
    asm volatile("lfence":::"memory");
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    tsc = ((unsigned long)a) | (((unsigned long)d) << 32);
#endif
    return tsc;
}


static int sum = 0;

static inline int access_memory(char* ptr) {
    unsigned long start = read_counter();
    char value = *ptr;
    unsigned long end = read_counter();
    sum += value;
    return end - start;
}

int main(int argc, char** argv) {

    int num_pages = 2000000;
    int page_size = 4096;
    int table_size = 1000;
    int count = 1000000;
    int stride = 731734319;

    unsigned int next = 0;

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

    initialize_counters();

    printf("Allocated!\n");

    int counts[table_size];
    for(int i=0; i<table_size; i++) counts[i]=0;

    for(int i=0; i<count; i++) {
        uint64_t time = access_memory(pages[next]);
        if(time>=table_size) time = table_size-1;
        counts[time]++;
        next = (next + stride) % num_pages;
    }

    for(int i=0; i<table_size; i++) {
        if (counts[i] > 0) {
            printf("%d, %d\n", i, counts[i]);
        }
    }

    return 0;
}

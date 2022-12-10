#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/syscall.h>

#include "utils.h"


/**
 * print an error message and then exit the process.
 * @param message the message to print
 */
void exit_with_error(const char* message) {
    perror(message);
    exit(-1);
}

/**
 * gets the realtime clock with nanosecond precision (if the system supports).
 * Getting the time using this call is slower than getting the TSC value.
 * So this is used only over longer durations.
 * @param ts a point to the timespec to be filled.
 */
void get_time(struct timespec* ts) {
    int result = clock_gettime(CLOCK_REALTIME, ts);
    if(result < 0) exit_with_error("clock_gettime");
}

/**
 * computes the difference (in nanos) between two timespec structs
 * @param start the start time
 * @param end  the end time
 * @return the nanos difference between end and start
 */
uint64_t elapsed_nanos(struct timespec* start, struct timespec* end) {
    uint64_t result = end->tv_sec - start->tv_sec;
    result *= ONE_BILLION;
    result += end->tv_nsec - start->tv_nsec;
    return result;
}

void print_heap_stack_address() {
    void* heap = malloc(1);
    if(heap == NULL) exit_with_error("malloc");
    uint64_t heap_addr = (uint64_t) heap;
    uint64_t stack_addr = (uint64_t) (&heap);
    printf("heap_addr:%016llx  stack_addr:%016llx\n", heap_addr, stack_addr);
}

#define __NR_tlbcount 441
unsigned long get_tlb_count(void)
{
    return syscall(__NR_tlbcount);
}


int is_prime(int n)
{
    if (n == 2 || n == 3)
        return 1;

    if (n <= 1 || n % 2 == 0 || n % 3 == 0)
        return 0;

    for (int i = 5; i * i <= n; i += 6)
    {
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    }

    return 1;
}

int find_next_prime(int n) {
    while(1) {
        if(is_prime(++n)) return n;
    }
}
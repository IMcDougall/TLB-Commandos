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



#define __NR_tlbcount 441
unsigned long get_tlb_count(void)
{
    return syscall(__NR_tlbcount);
}

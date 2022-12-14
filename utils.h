#include <stdint.h>

#ifndef TLB_UTILS_H
#define TLB_UTILS_H

#define ONE_BILLION (1000000000L)

/**
 * print an error message and then exit the process.
 * @param message the message to print
 */
void exit_with_error(const char* message);

/**
 * gets the realtime clock with nanosecond precision (if the system supports).
 * Getting the time using this call is slower than getting the TSC value.
 * So this is used only over longer durations.
 * @param ts a point to the timespec to be filled.
 */
void get_time(struct timespec* ts);

/**
 * computes the difference (in nanos) between two timespec structs
 * @param start the start time
 * @param end  the end time
 * @return the nanos difference between end and start
 */
uint64_t elapsed_nanos(struct timespec* start, struct timespec* end);

/**
 * get the number of tlb misses that occurred
 * @return tlb miss count
 */
unsigned long get_tlb_count(void);

/**
 * prints the address of a pointer in the heap and the stack.
 */
void print_heap_stack_address();

/**
 * find the next prime after n
 */
int find_next_prime(int n);


#endif //TLB_UTILS_H

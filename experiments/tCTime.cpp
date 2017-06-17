#include <string>
#include <time.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/time.h>

#include "fmt/format.h"

#define AMQP_MS_PER_S  1000
#define AMQP_US_PER_MS 1000
#define AMQP_NS_PER_S  1000000000
#define AMQP_NS_PER_MS 1000000
#define AMQP_NS_PER_US 1000

uint64_t gettime() {
	struct timespec tp;
	if (-1 == clock_gettime(CLOCK_MONOTONIC, &tp)) {
	    return 0;
	}
	return ((uint64_t)tp.tv_sec * AMQP_NS_PER_S + (uint64_t)tp.tv_nsec);
}

uint64_t elapsed_time(const uint64_t previous_time) {
	uint64_t diff = gettime() - previous_time;
	return diff;
}

struct timeval;
uint64_t timeval2ns(timeval *tp) {
	return ((uint64_t)tp->tv_sec * AMQP_NS_PER_S + (uint64_t)tp->tv_usec) * AMQP_NS_PER_US;
}

int main() {
	struct timeval timeout;
	uint64_t t1 = gettime();
	sleep(1);
	fmt::print("Elapsed time: {} ns\n", elapsed_time(t1));
	sleep(2);
	fmt::print("Elapsed time: {} ns\n", elapsed_time(t1));
}

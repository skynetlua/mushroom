#ifndef __mr_time_h__
#define __mr_time_h__

#include <stdint.h>

#include <time.h>
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

static inline void mr_timeofday(long *sec, long *usec){
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	static long mode = 0, addsec = 0;
	BOOL retval;
	static int64_t freq = 1;
	int64_t qpc;
	if (mode == 0) {
		retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		freq = (freq == 0)? 1 : freq;
		retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
		addsec = (long)time(NULL);
		addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
		mode = 1;
	}
	retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
	retval = retval * 2;
	if (sec) *sec = (long)(qpc / freq) + addsec;
	if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
#else
	struct timeval time;
	gettimeofday(&time, NULL);
	if (sec) *sec = time.tv_sec;
	if (usec) *usec = time.tv_usec;
#endif
}

static inline int64_t mr_clock64(void){
	long s, u;
	mr_timeofday(&s, &u);
	int64_t value = ((int64_t)s) * 1000 + (u / 1000);
	return value;
}

static inline uint32_t mr_clock(void){
	return (uint32_t)(mr_clock64() & 0xfffffffful);
}

static inline int64_t mr_time(void){
	long s, u;
	mr_timeofday(&s, &u);
	int64_t value = ((int64_t)s) * 1000 + (u / 1000);
	return value;
}

static inline void mr_sleep(unsigned long millisecond){
	#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
		Sleep(millisecond);
	#else
		// struct timespec ts;
		// ts.tv_sec = (time_t)(millisecond / 1000);
		// ts.tv_nsec = (long)((millisecond % 1000) * 1000000);
		// nanosleep(&ts, NULL);
		usleep((millisecond << 10) - (millisecond << 4) - (millisecond << 3));
	#endif
}

#endif



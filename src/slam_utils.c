/*
 * \file: slam_utils.c
 * \brief: Created by hushouguo at 16:51:19 Jul 25 2019
 */

#include "slam.h"

uint32_t slam_cpus() {
	long n = sysconf(_SC_NPROCESSORS_CONF);
	return n > 0 ? n : 1;
}

/* check that a floating point number is integer */
bool slam_is_integer(double value) {
	return value == (int64_t) value;
}


//   
// get current time milliseconds    
uint64_t slam_milliseconds() {
    // cost of executing 1 million times is:
    //      c++ 11 waste: 38 ms
    //      gettimeofday waste: 19 ms
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

//
// limits: stack_size, max_files


// limits, linux default stack size: 8M (soft), 4G (hard)
bool slam_set_limit_stack_size(uint32_t value) {
	struct rlimit limit;
	int rc = getrlimit(RLIMIT_STACK, &limit);
	CHECK_RETURN(rc == 0, false, "getrlimit error:%d,%s", errno, strerror(errno));
	limit.rlim_cur = value;
	rc = setrlimit(RLIMIT_STACK, &limit);
	CHECK_RETURN(rc == 0, false, "setrlimit error:%d,%s", errno, strerror(errno));
	return true;
}

uint32_t slam_get_limit_stack_size() {
	struct rlimit limit;
	int rc = getrlimit(RLIMIT_STACK, &limit);
	CHECK_RETURN(rc == 0, 0, "getrlimit error:%d,%s", errno, strerror(errno));
	return limit.rlim_cur;
}

// limits, linux default open files: 1024 (soft), 4096 (hard)
bool slam_set_limit_open_files(uint32_t value) {
	struct rlimit limit;
	int rc = getrlimit(RLIMIT_NOFILE, &limit);
	CHECK_RETURN(rc == 0, false, "getrlimit error:%d,%s", errno, strerror(errno));
	limit.rlim_cur = value;
	rc = setrlimit(RLIMIT_NOFILE, &limit);
	CHECK_RETURN(rc == 0, false, "setrlimit error:%d,%s", errno, strerror(errno));
	return true;
}

uint32_t slam_get_limit_open_files() {
	struct rlimit limit;
	int rc = getrlimit(RLIMIT_NOFILE, &limit);
	CHECK_RETURN(rc == 0, 0, "getrlimit error:%d,%s", errno, strerror(errno));
	return limit.rlim_cur;
}


/*
 * \file: test_easylog.cpp
 * \brief: Created by hushouguo at 15:55:41 Sep 06 2018
 */

#include <vector>
#include "Easylog.h"

using namespace logger;

using u8 	=		uint8_t;
using u16 	=		uint16_t;
using u32	=		uint32_t;
using u64	=		uint64_t;
using s8	=		int8_t;
using s16	=		int16_t;
using s32	=		int32_t;
using s64	=		int64_t;

//
// get current time milliseconds	
u64 currentMillisecond() {
	// cost of executing 1 million times is:
	// 		c++ 11 waste: 38 ms
	//		gettimeofday waste: 19 ms
#if true
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
#endif
}

void test_easylog() {
	u8  val_u8 = 65;
	s8  val_s8 = '*';
	u16 val_u16 = 100;
	s16 val_s16 = -100;
	u32 val_u32 = 1000;
	s32 val_s32 = -1000;
	u64 val_u64 = 1234567890000;
	s64 val_s64 = -1234567890000;
	char val_char = 'a';
	bool val_bool = true;
	float val_float = 1.23456;
	double val_double = -2.345351;
	const char* val_cstr = "i am a const string";
	std::string val_string = "i am a string";

	Easylog::syslog()->set_level(GLOBAL);
	Easylog::syslog()->set_tostdout(GLOBAL, true);

	Trace << "u8: " << val_u8;
	Trace << "s8: " << val_s8;
	Trace << "u16: " << val_u16;
	Trace << "s16: " << val_s16;
	Trace << "u32: " << val_u32;
	Trace << "s32: " << val_s32;
	Trace << "u64: " << val_u64;
	Trace << "s64: " << val_s64;
	Trace << "char: " << val_char;
	Trace << "bool: " << val_bool;
	Trace << "float: " << val_float;
	Trace << "double: " << val_double;
	Trace << "cstr: " << val_cstr;
	Trace << "string: " << val_string;

	Debug << "i am a debug log";
	Trace << "i am a trace log";
	Alarm << "i am a alarm log";
	Error << "i am a error log";
	//Panic << "i am a panic log";
	System << "i am a system log";

	sleep(2);
	
	Easylog::syslog()->set_autosplit_day(true);
	Easylog::syslog()->set_destination(".logs");
	Easylog::syslog()->set_tostdout(GLOBAL, false);

	auto performance_single_thread = [](int times) {
		for (int i = 0; i < times; ++i) {
			Trace << "For the average case, it is not always a big difference between the two. " << i;
		}
	};

	auto performance_multi_thread = [](int threads, int times) {
		std::vector<std::thread*> v;
		for (int i = 0; i < threads; ++i) {
			v.push_back(new std::thread([i, times](){
				int j = 0;
				for (; j < times; ++j) {
					Trace << "[" << i << "] For the average case, it is not always a big difference between the two. " << j;
				}
			}));
		}
		for (auto& t : v) {
			if (t->joinable()) {
				t->join();
			}
		}
	};

	if (true) {
		Easylog::syslog()->set_tofile(GLOBAL, "tests-single");
		u64 t1 = currentMillisecond();
		performance_single_thread(1000000);
		u64 t2 = currentMillisecond();
		fprintf(stderr, "performance: single thread cost milliseconds: %ld, times: 1,000,000\n", t2 - t1);
	}

	if (true) {
		Easylog::syslog()->set_tofile(GLOBAL, "tests-multi");
		u64 t1 = currentMillisecond();
		performance_multi_thread(8, 500000);
		u64 t2 = currentMillisecond();
		fprintf(stderr, "performance: 8 threads cost milliseconds: %ld, times: 500,000\n", t2 - t1);
	}

	//Easylog::syslog()->stop();

	System << "easylog test OK";
}

int main() {
	test_easylog();
	Easylog::syslog()->stop();
	return 0;
}

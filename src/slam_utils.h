/*
 * \file: slam_utils.h
 * \brief: Created by hushouguo at 16:51:18 Jul 25 2019
 */
 
#ifndef __SLAM_UTILS_H__
#define __SLAM_UTILS_H__

extern const char* errstring(int err);
extern uint32_t slam_cpus();
extern bool slam_is_integer(double value);
extern uint64_t slam_seconds(); 
extern uint64_t slam_milliseconds();

// seconds = 0: current seconds
// time_format is nullptr: default value is "%y/%02m/%02d %02H:%02M:%02S", like: 18/06/29 15:04:18
extern const char* slam_timestamp(char* buf, size_t bufsize, uint64_t seconds, const char* time_format);
// format like: Sat, 11 Mar 2017 21:49:51 GMT
extern const char* slam_timestamp_gmt(char* buf, size_t bufsize, uint64_t seconds);

// hash string
extern uint32_t slam_hash_string(const char* s);

// io
extern bool slam_is_dir(const char* file);
extern bool slam_file_exist(const char* file);
extern bool slam_file_readable(const char* file);
extern bool slam_file_writable(const char* file);
extern bool slam_file_accessable(const char* file);
extern uint64_t slam_file_size(const char* filename);

// process title
extern void slam_set_process_title(int argc, char* argv[], const char* title);

extern const char* slam_program_name();



// limits, linux default stack size: 8M (soft), 4G (hard)
extern bool slam_set_limit_stack_size(uint32_t value);
extern uint32_t slam_get_limit_stack_size();

// limits, linux default open files: 1024 (soft), 4096 (hard)
extern bool slam_set_limit_open_files(uint32_t value);
extern uint32_t slam_get_limit_open_files();

// dump info for allocate memory
extern void slam_mallinfo();

#endif

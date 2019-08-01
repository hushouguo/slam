/*
 * \file: slam_utils.h
 * \brief: Created by hushouguo at 16:51:18 Jul 25 2019
 */
 
#ifndef __SLAM_UTILS_H__
#define __SLAM_UTILS_H__

extern uint32_t slam_cpus();
extern bool slam_is_integer(double value);
extern uint64_t slam_milliseconds();

// limits, linux default stack size: 8M (soft), 4G (hard)
extern bool slam_set_limit_stack_size(uint32_t value);
extern uint32_t slam_get_limit_stack_size();

// limits, linux default open files: 1024 (soft), 4096 (hard)
extern bool slam_set_limit_open_files(uint32_t value);
extern uint32_t slam_get_limit_open_files();

#endif

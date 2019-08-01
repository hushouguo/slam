/*
 * \file: slam_main.h
 * \brief: Created by hushouguo at 11:00:54 Jul 30 2019
 */
 
#ifndef __SLAM_MAIN_H__
#define __SLAM_MAIN_H__

struct slam_main_s {
	bool halt;
	bool standalone;

	/* reload flags */
	bool reload_dynamic_lib;
	bool reload_lua;

	/* limits */
	size_t limit_stack_size, limit_max_files;

	/* the number of worker */
	size_t worker_number;

	/* security */
	size_t conn_max_silent;
	size_t conn_check_interval, conn_max_messages;

	/* main runnable */
	slam_runnable_t* runnable;
};

typedef struct slam_main_s slam_main_t;

extern slam_main_t* slam_main();

#endif

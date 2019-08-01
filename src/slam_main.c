/*
 * \file: slam_main.c
 * \brief: Created by hushouguo at 11:17:03 Jul 30 2019
 */

#include "slam.h"

#define SLAM_DEF_LIMIT_STACK_SIZE		16*MB	// limits, linux default stack size: 8M (soft), 4G (hard)
#define SLAM_DEF_LIMIT_OPEN_FILES		8192	// limits, linux default open files: 1024 (soft), 4096 (hard)
#define SLAM_DEF_CONN_MAX_SILENT		10		// seconds
#define SLAM_DEF_CONN_CHECK_INTERVAL	10		// seconds
#define SLAM_DEF_CONN_MAX_MESSAGES		300		// the number of messages

static slam_main_t* __slam_main = nullptr;

__attribute__((constructor)) static void __slam_main_init() {
	if (!__slam_main) {
		__slam_main = (slam_main_t *) malloc(sizeof(slam_main_t));
		
		__slam_main->halt = false;
		__slam_main->standalone = true;
		
		__slam_main->reload_dynamic_lib = false;
		__slam_main->reload_lua = false;
		
		__slam_main->limit_stack_size = SLAM_DEF_LIMIT_STACK_SIZE;
		__slam_main->limit_max_files = SLAM_DEF_LIMIT_OPEN_FILES;

		__slam_main->worker_number = slam_cpus();

		__slam_main->conn_max_silent = SLAM_DEF_CONN_MAX_SILENT;
		__slam_main->conn_check_interval = SLAM_DEF_CONN_CHECK_INTERVAL;
		__slam_main->conn_max_messages = SLAM_DEF_CONN_MAX_MESSAGES;

		__slam_main->runnable = slam_runnable_new();
	}
}

__attribute__((destructor)) static void __slam_main_destroy() {
	slam_free(__slam_main);
}

slam_main_t* slam_main() {
	return __slam_main;
}


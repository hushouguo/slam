/*
 * \file: slam_main.h
 * \brief: Created by hushouguo at 11:00:54 Jul 30 2019
 */
 
#ifndef __SLAM_MAIN_H__
#define __SLAM_MAIN_H__

#define SLAM_VERSION_MAJOR		0
#define SLAM_VERSION_MINOR		1
#define SLAM_VERSION_PATCH		0

struct slam_main_s {
	bool halt;
	bool runasdaemon;
	bool runasmaster;

	/* argc, argv */
	int argc;
	char** argv;

	/* reload flags */
	bool reload_entryfile;

	/* lua entryfile */
	const char* entryfile;

	/* logfile */
	const char* logfile;

	/* limits */
	size_t limit_stack_size, limit_max_files;

	/* the number of worker */
	uint32_t worker_number;

	/* security */
	size_t conn_max_silent;
	size_t conn_check_interval, conn_max_messages;

    /* poll number of events */
    size_t poll_event_number;

    /* max size of package */
    size_t max_package_size;
    size_t message_queue_size;

    /* log */
    slam_log_t* log;
    
    /* global message queue */
    slam_message_queue_t* mq;
    
	/* main runnable */
	slam_runnable_t* runnable;

	/* io thread */
	slam_io_thread_t* io;
};

typedef struct slam_main_s slam_main_t;

extern slam_main_t* __slam_main;

extern bool slam_main_init(int argc, char** argv);
extern void slam_main_delete();
extern void slam_main_run();

#endif

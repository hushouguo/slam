/*
 * \file: slam_runnable.h
 * \brief: Created by hushouguo at 16:39:10 Jul 25 2019
 */
 
#ifndef __SLAM_RUNNABLE_H__
#define __SLAM_RUNNABLE_H__

struct slam_runnable_s {
	slam_lua_t* lua;
	slam_poll_t* poll;
	slam_protocol_t* protocol;
	slam_timer_list_t* timer_list;
};

typedef struct slam_runnable_s slam_runnable_t;

extern slam_runnable_t* slam_runnable_new();
extern void slam_runnable_delete(slam_runnable_t* runnable);

extern void slam_runnable_wakeup(slam_runnable_t* runnable);
extern void slam_runnable_run(slam_runnable_t* runnable);

// load & reload entryfile
extern bool slam_runnable_load_entryfile(slam_runnable_t* runnable, const char* entryfile);

// add & remove timer
extern slam_timer_t* slam_runnable_add_timer(slam_runnable_t* runnable, uint64_t interval, bool forever, int ref, slam_lua_value_t* ctx);
extern void slam_runnable_remove_timer(slam_runnable_t* runnable, int ref);

#endif


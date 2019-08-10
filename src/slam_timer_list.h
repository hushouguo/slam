/*
 * \file: slam_timer_list.h
 * \brief: Created by hushouguo at 14:43:44 Aug 01 2019
 */
 
#ifndef __SLAM_TIMER_LIST_H__
#define __SLAM_TIMER_LIST_H__

//
// We don't need that complicated timer, We don't have many timers either.
// So we use double-direct ordered list to deal with it.
//
struct slam_timer_s {
	uint64_t interval;		// milliseconds
	uint64_t expire_time;	// expire time
	bool forever;
	int ref;	// reference to lua closure
	slam_lua_value_t* ctx;
	struct slam_timer_s *prev, *next;
};
typedef struct slam_timer_s slam_timer_t;

extern slam_timer_t* slam_timer_new(uint64_t interval, bool forever, int ref, slam_lua_value_t* ctx);

struct slam_timer_list_s {
	slam_timer_t* timer_head_node;
};
typedef struct slam_timer_list_s slam_timer_list_t;

extern slam_timer_list_t* slam_timer_list_new();
extern void slam_timer_list_delete(slam_timer_list_t* timer_list);

extern void slam_timer_list_add(slam_timer_list_t* timer_list, slam_timer_t* timer_newnode);
slam_timer_t* slam_timer_list_find(slam_timer_list_t* timer_list, int ref);
extern void slam_timer_list_remove_by_ref(slam_timer_list_t* timer_list, int ref, bool remove_timer);
extern void slam_timer_list_remove_by_node(slam_timer_list_t* timer_list, slam_timer_t* timer_node, bool remove_timer);
extern int64_t slam_timer_list_min_interval(slam_timer_list_t* timer_list); // -1: no more timer

typedef struct slam_runnable_s slam_runnable_t;
extern void slam_timer_list_run(slam_runnable_t* runnable);

#endif

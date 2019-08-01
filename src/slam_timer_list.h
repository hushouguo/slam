/*
 * \file: slam_timer_list.h
 * \brief: Created by hushouguo at 14:43:44 Aug 01 2019
 */
 
#ifndef __SLAM_TIMER_LIST_H__
#define __SLAM_TIMER_LIST_H__

typedef struct slam_timer_s slam_timer_t;

extern slam_timer_t* slam_timer_new(uint64_t interval, bool forever, int ref, slam_lua_value_t* ctx);
extern uint32_t slam_timer_id(slam_timer_t* timer_node);
extern int slam_timer_ref(slam_timer_t* timer_node);
extern slam_lua_value_t* slam_timer_ctx(slam_timer_t* timer_node);

typedef struct slam_timer_list_s slam_timer_list_t;

extern slam_timer_list_t* slam_timer_list_new();
extern void slam_timer_list_delete(slam_timer_list_t* timer_list);

extern void slam_timer_list_add(slam_timer_list_t* timer_list, slam_timer_t* timer_newnode);
extern void slam_timer_list_remove_by_id(slam_timer_list_t* timer_list, uint32_t timerid, bool remove_timer);
extern void slam_timer_list_remove_by_node(slam_timer_list_t* timer_list, slam_timer_t* timer_node, bool remove_timer);
extern int64_t slam_timer_list_min_interval(slam_timer_list_t* timer_list); // -1: no more timer

typedef struct slam_runnable_s slam_runnable_t;
extern void slam_timer_list_run(slam_runnable_t* runnable);


#define TIMER_LIST_ADD_FRONT(timer_node, timer_newnode) \
	do {\
		(timer_newnode)->next = timer_node, (timer_newnode)->prev = (timer_node)->prev;\
		if ((timer_node)->prev) {\
			(timer_node)->prev->next = timer_newnode;\
		}\
		(timer_node)->prev = timer_newnode;\
	} while (false)

#define TIMER_LIST_ADD_BACK(timer_node, timer_newnode) \
	do {\
		(timer_newnode)->prev = timer_node, (timer_newnode)->next = timer_node;\
		if ((timer_node)->next) {\
			(timer_node)->next->prev = timer_newnode;\
		}\
		(timer_node)->next = timer_newnode;\
	} while (false)

#define TIMER_LIST_REMOVE(timer_node) \
	do {\
		if ((timer_node)->prev) {\
			(timer_node)->prev->next = (timer_node)->next;\
		}\
		if ((timer_node)->next) {\
			((timer_node)->next->prev = (timer_node)->prev;\
		}\
		TIMER_NODE_INIT(timer_node);\
	} while (false)

#endif

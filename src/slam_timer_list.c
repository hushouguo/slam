/*
 * \file: slam_timer_list.c
 * \brief: Created by hushouguo at 14:44:14 Aug 01 2019
 */

#include "slam.h"

//
// We don't need that complicated timer, We don't have many timers either.
// So we use double-direct ordered list to deal with it.
//
struct slam_timer_s {
	uint32_t id;
	uint64_t interval;		// milliseconds
	uint64_t expire_time;	// expire time
	bool forever;
	int ref;	// reference to lua closure
	slam_lua_value_t* ctx;
	struct slam_timer_s *prev, *next;
};

static uint32_t __timer_baseid = 1;

slam_timer_t* slam_timer_new(uint64_t interval, bool forever, int ref, slam_lua_value_t* ctx) {
	slam_timer_t* timer_node = (slam_timer_t *) malloc(sizeof(slam_timer_t));
	timer_node->id = __timer_baseid++;
	timer_node->interval = interval;
	timer_node->expire_time = slam_milliseconds() + interval;
	timer_node->forever = forever;
	timer_node->ref = ref;
	timer_node->ctx = ctx;
	timer_node->prev = timer_node->next = nullptr;
	return timer_node;
}

void slam_timer_delete(slam_timer_t* timer_node) {
	if (timer_node->ctx) {
		slam_lua_value_delete(timer_node->ctx);
	}
	//TODO: remove ref from lua
	slam_free(timer_node);
}

uint32_t slam_timer_id(slam_timer_t* timer_node) {
	return timer_node->id;
}

int slam_timer_ref(slam_timer_t* timer_node) {
	return timer_node->ref;
}

slam_lua_value_t* slam_timer_ctx(slam_timer_t* timer_node) {
	return timer_node->ctx;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

struct slam_timer_list_s {
	slam_timer_t* timer_head_node;
};

void slam_timer_list_dump(slam_timer_list_t* timer_list, const char* prefix) {
	slam_timer_t* timer_node = timer_list->timer_head_node;
	if (prefix) {
		Debug("%s", prefix);
	}
	while (timer_node != nullptr) {
		Debug("    timer: %d, interval: %ld, expire_time: %ld, forever: %d, ref: %d",
			timer_node->id,
			timer_node->interval,
			timer_node->expire_time,
			timer_node->forever,
			timer_node->ref
		);
		timer_node = timer_node->next;
	}	
}

slam_timer_list_t* slam_timer_list_new() {
	slam_timer_list_t* timer_list = (slam_timer_list_t *) malloc(sizeof(slam_timer_list_t));
	timer_list->timer_head_node = nullptr;
	return timer_list;
}

void slam_timer_list_delete(slam_timer_list_t* timer_list) {
	slam_timer_t* timer_node = timer_list->timer_head_node;
	while (timer_node != nullptr) {
		slam_timer_t* timer_next_node = timer_node->next;
		slam_timer_delete(timer_node);
		timer_node = timer_next_node;
	}
	slam_free(timer_list);
}

void slam_timer_list_add(slam_timer_list_t* timer_list, slam_timer_t* timer_newnode) {
	if (timer_list->timer_head_node == nullptr) {
		timer_list->timer_head_node = timer_newnode;
		timer_newnode->prev = timer_newnode->next = nullptr;
	}
	else {
		slam_timer_t* timer_node = timer_list->timer_head_node;
		while (timer_node != nullptr) {
			// push_front
			if (timer_newnode->expire_time < timer_node->expire_time) {
				timer_newnode->next = timer_node;
				timer_newnode->prev = timer_node->prev;
				if (timer_node->prev) {
					timer_node->prev->next = timer_newnode;
				}
				timer_node->prev = timer_newnode;
				if (timer_node == timer_list->timer_head_node) {
					timer_list->timer_head_node = timer_newnode;
				}
				break;
			}
			// push_back
			else if (timer_node->next == nullptr) {
				timer_newnode->prev = timer_node;
				timer_newnode->next = nullptr;
				timer_node->next = timer_newnode;
				break;
			}
			else {
				timer_node = timer_node->next;
			}
		}
	}
}

slam_timer_t* slam_timer_list_find(slam_timer_list_t* timer_list, uint32_t timerid) {
	slam_timer_t* timer_node = timer_list->timer_head_node;
	while (timer_node != nullptr && timer_node->id != timerid) {
		timer_node = timer_node->next;
	}
	return timer_node;
}

void slam_timer_list_remove_by_id(slam_timer_list_t* timer_list, uint32_t timerid, bool remove_timer) {
	slam_timer_t* timer_node = slam_timer_list_find(timer_list, timerid);
	CHECK_RETURN(timer_node != nullptr, (void) 0, "not found timer: %d", timerid);
	slam_timer_list_remove_by_node(timer_list, timer_node, remove_timer);
}

void slam_timer_list_remove_by_node(slam_timer_list_t* timer_list, slam_timer_t* timer_node, bool remove_timer) {
	if (timer_list->timer_head_node == timer_node) {
		timer_list->timer_head_node = timer_list->timer_head_node->next;
		if (timer_list->timer_head_node) {
			timer_list->timer_head_node->prev = nullptr;
		}
	}
	else {
		assert(timer_node->prev != nullptr);
		timer_node->prev->next = timer_node->next;
		if (timer_node->next != nullptr) {
			timer_node->next->prev = timer_node->prev;
		}
	}
	if (remove_timer) {
		slam_timer_delete(timer_node);
	}
}

int64_t slam_timer_list_min_interval(slam_timer_list_t* timer_list) {
	uint64_t nowtime = slam_milliseconds();
	slam_timer_t* timer_node = timer_list->timer_head_node;
	return timer_node == nullptr ? -1 : (
		timer_node->expire_time > nowtime ? (timer_node->expire_time - nowtime) : -1
		);	
}

void slam_timer_list_run(slam_runnable_t* runnable) {
	uint64_t nowtime = slam_milliseconds();
	slam_timer_list_t* timer_list = slam_runnable_timer_list(runnable);
	
	slam_timer_list_dump(timer_list, "run begin");
	while (timer_list->timer_head_node != nullptr && timer_list->timer_head_node->expire_time <= nowtime) {
		slam_timer_t* timer_node = timer_list->timer_head_node;
		slam_timer_list_remove_by_node(timer_list, timer_node, false);
		
		// trigger timer event to lua
		slam_lua_event_timer(runnable, timer_node);
		
		if (timer_node->forever) {
			timer_node->expire_time = nowtime + timer_node->interval;
			slam_timer_list_add(timer_list, timer_node);
		}
		else {
			slam_timer_delete(timer_node);
		}
	}	
	slam_timer_list_dump(timer_list, "run end");
}


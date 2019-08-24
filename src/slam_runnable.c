/*
 * \file: slam_runnable.c
 * \brief: Created by hushouguo at 16:39:13 Jul 25 2019
 */

#include "slam.h"

slam_runnable_t* slam_runnable_new() {
	slam_runnable_t* runnable = (slam_runnable_t *) slam_malloc(sizeof(slam_runnable_t));
	runnable->lua = nullptr;
	runnable->poll = slam_poll_new();
	if (!slam_poll_add_socket(runnable->poll, __slam_main->fd_io_sockets[SLAM_FD_WORKER])) {
		return nullptr;
	}	
	if (!slam_poll_add_socket(runnable->poll, __slam_main->fd_worker_sockets[SLAM_FD_WORKER])) {
		return nullptr;
	}
	runnable->protocol = slam_protocol_new();
	runnable->timer_list = slam_timer_list_new();
	return runnable;
}

void slam_runnable_delete(slam_runnable_t* runnable) {
	if (runnable->lua) {
		slam_lua_event_stop(runnable); /* notify event.stop to lua */
		slam_lua_delete(runnable->lua);
	}
	slam_poll_delete(runnable->poll);
	slam_timer_list_delete(runnable->timer_list);
	if (runnable->protocol) {
	    slam_protocol_delete(runnable->protocol);
	}
	slam_free(runnable);
}

// load & reload entryfile and protocol lib
bool slam_runnable_load_entryfile(slam_runnable_t* runnable, const char* entryfile) {
	if (runnable->lua) {
		slam_lua_delete(runnable->lua);
	}
	runnable->lua = slam_lua_new(SLAM_LUA_STACK_SIZE);
	if (runnable->protocol) {
	    slam_protocol_delete(runnable->protocol);
	}
	runnable->protocol = slam_protocol_new();
	if (!slam_lua_load_entryfile(runnable->lua, entryfile)) {
		return false;
	}
	slam_lua_event_start(runnable);
	return true;
}

void slam_runnable_wakeup(slam_runnable_t* runnable) {
    slam_poll_set_socket_pollout(runnable->poll, __slam_main->fd_io_sockets[SLAM_FD_WORKER], true);
}

void slam_runnable_do_message(slam_runnable_t* runnable) {
    while (!slam_message_queue_empty(__slam_main->mq)) {
        slam_message_t* message = slam_message_queue_pop_front(__slam_main->mq);
        if (message->type == MESSAGE_TYPE_PROTOBUF_PACKAGE) {
            slam_protocol_decode(runnable, message);
        }
        else if (message->type == MESSAGE_TYPE_ESTABLISH_CONNECTION) {
            slam_lua_event_establish_connection(runnable, message->fd);
        }
        else if (message->type == MESSAGE_TYPE_LOST_CONNECTION) {
            slam_lua_event_lost_connection(runnable, message->fd);
        }
        else {
            log_error("illegal message->type: %d", message->type);
        }
        slam_message_delete(message);
    }
}

void slam_runnable_run(slam_runnable_t* runnable) {
    while (!__slam_main->halt) {
        slam_poll_event events[__slam_main->poll_event_number];
        int64_t timeout = slam_timer_list_min_interval(runnable->timer_list);
        ssize_t i, events_number = slam_poll_wait(runnable->poll, events, __slam_main->poll_event_number, timeout);
        (void)(i);
        (void)(events_number);
        if (__slam_main->halt) {
            return; // after waking up, check the exit mark first
        }
        slam_runnable_do_message(runnable);
    	slam_timer_list_run(runnable);
	}
}

slam_timer_t* slam_runnable_add_timer(slam_runnable_t* runnable, uint64_t interval, bool forever, int ref, slam_lua_value_t* ctx) {
	slam_timer_t* timer_newnode = slam_timer_new(interval, forever, ref, ctx);
	slam_timer_list_add(runnable->timer_list, timer_newnode);
	//slam_lua_dump_registry_table(runnable->lua);
	return timer_newnode;
}

void slam_runnable_remove_timer(slam_runnable_t* runnable, int ref) {
    slam_timer_t* timer_node = slam_timer_list_find(runnable->timer_list, ref);
    CHECK_RETURN(timer_node != nullptr, (void) 0, "not found timer: %d", ref);
    slam_lua_unref(runnable->lua, timer_node->ref);
    slam_timer_list_remove_by_node(runnable->timer_list, timer_node, true);
	//slam_lua_dump_registry_table(runnable->lua);
}


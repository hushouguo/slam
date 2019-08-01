/*
 * \file: slam_runnable.c
 * \brief: Created by hushouguo at 16:39:13 Jul 25 2019
 */

#include "slam.h"

#define MAX_SOCKET_NUMBER		65536
#define VALID_SOCKET(fd)		((fd) >= 0 && (fd) < MAX_SOCKET_NUMBER)
#define ASSERT_SOCKET(fd)		assert(VALID_SOCKET(fd))

struct slam_runnable_s {
	slam_poll_t* poll;
	slam_socket_t* sockets[MAX_SOCKET_NUMBER];
	slam_protocol_t* protocol;
	slam_lua_t* lua;
	slam_timer_list_t* timer_list;
};

slam_runnable_t* slam_runnable_new() {
	slam_runnable_t* runnable = (slam_runnable_t *) malloc(sizeof(slam_runnable_t));
	runnable->poll = slam_poll_new();
	memset(runnable->sockets, 0, sizeof(runnable->sockets));
	runnable->protocol = nullptr;
	runnable->lua = nullptr;
	runnable->timer_list = slam_timer_list_new();
	return runnable;
}

void slam_runnable_delete(slam_runnable_t* runnable) {
	int fd;
	if (runnable->lua) {
		slam_lua_event_stop(runnable); /* notify event.stop to lua */
		slam_lua_delete(runnable->lua);
	}
	slam_poll_delete(runnable->poll);
	for (fd = 0; fd < MAX_SOCKET_NUMBER; ++fd) {
		if (runnable->sockets[fd] != nullptr) {
			slam_socket_delete(runnable->sockets[fd]);
		}
	}
	slam_timer_list_delete(runnable->timer_list);
	if (runnable->protocol) {
		slam_protocol_delete(runnable->protocol);
	}
	slam_free(runnable);
}

bool slam_runnable_load_entryfile(slam_runnable_t* runnable, const char* entryfile) {
	if (runnable->lua) {
		slam_lua_delete(runnable->lua);
	}
	runnable->lua = slam_lua_new(SLAM_LUA_STACK_SIZE);
	if (!slam_lua_load_entryfile(runnable->lua, entryfile)) {
		slam_lua_delete(runnable->lua);
		return false;
	}
	slam_lua_event_start(runnable);
	return true;
}

bool slam_runnable_load_protocol(slam_runnable_t* runnable, const char* dynamic_lib_name) {
	if (runnable->protocol) {
		slam_protocol_delete(runnable->protocol);
	}
	runnable->protocol = slam_protocol_new();
	if (!slam_protocol_load_dynamic_lib(runnable->protocol, dynamic_lib_name) ) {
		slam_protocol_delete(runnable->protocol);
		return false;
	}
	return true;
}

bool slam_runnable_reload_entryfile(slam_runnable_t* runnable, const char* entryfile) {
	return runnable->lua ? slam_lua_reload_entryfile(runnable->lua, entryfile) : slam_runnable_load_protocol(runnable, entryfile);
}

bool slam_runnable_reload_protocol(slam_runnable_t* runnable, const char* dynamic_lib_name) {
	return runnable->protocol ? slam_protocol_reload_dynamic_lib(runnable->protocol, dynamic_lib_name) : slam_runnable_load_protocol(runnable, dynamic_lib_name);
}

bool slam_runnable_add_socket(slam_runnable_t* runnable, slam_socket_t* newsocket) {
	ASSERT_SOCKET(newsocket->fd);
	assert(runnable->sockets[newsocket->fd] == nullptr);
	if (!slam_poll_add_socket(runnable->poll, newsocket->fd)) {
		return false;
	}
	runnable->sockets[newsocket->fd] = newsocket;
	slam_lua_event_establish_connection(runnable, newsocket->fd);
	return true;
}

void slam_runnable_remove_socket(slam_runnable_t* runnable, SOCKET fd) {
	ASSERT_SOCKET(fd);
	if (runnable->sockets[fd] == nullptr) {
		return;
	}
	slam_poll_remove_socket(runnable->poll, fd);
	slam_socket_delete(runnable->sockets[fd]);
	runnable->sockets[fd] = nullptr;
	slam_lua_event_lost_connection(runnable, fd);
}

void slam_runnable_accept_socket(slam_runnable_t* runnable, slam_socket_t* socket) {
	assert(socket->type == SOCKET_SERVER);
	while (true) {
		slam_socket_t* newsocket = slam_socket_accept(socket);
		if (!newsocket) {
			return; /* EAGAIN */
		}
		if (!slam_runnable_add_socket(runnable, newsocket)) {
			slam_socket_delete(newsocket);
		}
	}
}

void slam_runnable_read_socket(slam_runnable_t* runnable, SOCKET fd) {
	slam_socket_t* socket = nullptr;
	ASSERT_SOCKET(fd);
	if (runnable->sockets[fd] == nullptr) {
		return; /* already close */
	}
	socket = runnable->sockets[fd];
	if (socket->type == SOCKET_SERVER) {
		slam_runnable_accept_socket(runnable, socket);
	}
	else {		
		if (slam_socket_read(socket) < 0 || !slam_protocol_decode(runnable, socket)) {
			slam_runnable_remove_socket(runnable, fd);
		}
	}
}

void slam_runnable_write_socket(slam_runnable_t* runnable, SOCKET fd) {
	slam_socket_t* socket = nullptr;
	ASSERT_SOCKET(fd);
	if (runnable->sockets[fd] == nullptr) {
		return; /* already close */
	}
	socket = runnable->sockets[fd];
	if (socket->type != SOCKET_SERVER) {
		if (slam_socket_write_buffer(socket) < 0) {
			slam_runnable_remove_socket(runnable, fd);
		}
	}
}

void slam_runnable_do_message(slam_runnable_t* runnable, int timeout) {
	slam_poll_event events[128];
	ssize_t i, events_number = slam_poll_wait(runnable->poll, events, 128, timeout);
	for (i = 0; i < events_number; ++i) {
		slam_poll_event* event = &events[i];
		if (event->events & EPOLLERR) {
			CHECK_RETURN(false, (void)0, "poll error: %d,%s", errno, strerror(errno));
			slam_runnable_remove_socket(runnable, event->data.fd);
		}
		else if (event->events & EPOLLHUP) {
			CHECK_RETURN(false, (void)0, "poll hup: %d,%s", errno, strerror(errno));
			slam_runnable_remove_socket(runnable, event->data.fd);
		}
		else if (event->events & EPOLLRDHUP) {
			CHECK_RETURN(false, (void)0, "poll rdhup: %d,%s", errno, strerror(errno));
			slam_runnable_remove_socket(runnable, event->data.fd);
		}
		else {
			if (event->events & EPOLLIN) {
				slam_runnable_read_socket(runnable, event->data.fd);
			}
			if (event->events & EPOLLOUT) {
				slam_runnable_write_socket(runnable, event->data.fd);
			}
		}
	}
}

void slam_runnable_run(slam_runnable_t* runnable) {	
	slam_runnable_do_message(runnable, slam_timer_list_min_interval(runnable->timer_list));
	slam_timer_list_run(runnable);
}

slam_socket_t* slam_runnable_newserver(slam_runnable_t* runnable, const char* address, int port) {
	slam_socket_t* socket = slam_socket_new();
	bool rc = slam_socket_bind(socket, address, port);
	if (!rc) {
		slam_socket_delete(socket);
		return nullptr;
	}
	if (!slam_runnable_add_socket(runnable, socket)) {
		slam_socket_delete(socket);
	}	
	return socket;
}

slam_socket_t* slam_runnable_newclient(slam_runnable_t* runnable, const char* address, int port, int timeout) {
	slam_socket_t* socket = slam_socket_new();
	bool rc = slam_socket_connect(socket, address, port, timeout);
	if (!rc) {
		slam_socket_delete(socket);
		return nullptr;
	}
	if (!slam_runnable_add_socket(runnable, socket)) {
		slam_socket_delete(socket);
	}	
	return socket;
}

slam_lua_t* slam_runnable_lua(slam_runnable_t* runnable) {
	return runnable->lua;
}

slam_protocol_t* slam_runnable_protocol(slam_runnable_t* runnable) {
	return runnable->protocol;
}

slam_socket_t* slam_runnable_socket(slam_runnable_t* runnable, SOCKET fd) {
	return fd >= 0 && fd < MAX_SOCKET_NUMBER ? runnable->sockets[fd] : nullptr;
}

slam_timer_list_t* slam_runnable_timer_list(slam_runnable_t* runnable) {
	return runnable->timer_list;
}

slam_timer_t* slam_runnable_add_timer(slam_runnable_t* runnable, uint64_t interval, bool forever, int ref, slam_lua_value_t* ctx) {
	slam_timer_t* timer_newnode = slam_timer_new(interval, forever, ref, ctx);
	slam_timer_list_add(runnable->timer_list, timer_newnode);
	return timer_newnode;
}

void slam_runnable_remove_timer(slam_runnable_t* runnable, uint32_t timerid) {
	slam_timer_list_remove_by_id(runnable->timer_list, timerid, true);
}


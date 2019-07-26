/*
 * \file: slam_runnable.c
 * \brief: Created by hushouguo at 16:39:13 Jul 25 2019
 */

#include "slam.h"

#define MAX_SOCKET_NUMBER		65536

struct slam_runnable_s {
	const char* entryfile;
	lua_State* L;
	slam_poll_t* poll;
	slam_socket_t* sockets[MAX_SOCKET_NUMBER];
};

slam_runnable_t* slam_runnable_new(const char* entryfile) {
	slam_runnable_t* runnable = (slam_runnable_t *) malloc(sizeof(slam_runnable_t));
	runnable->entryfile = strdup(entryfile);
	runnable->L = slam_lua_newstate(SLAM_LUA_STACK_SIZE);
	runnable->poll = slam_poll_new();
	memset(runnable->sockets, 0, sizeof(runnable->sockets));
	if (!slam_lua_dofile(runnable->L, runnable->entryfile)) {
		slam_runnable_delete(runnable);
		return nullptr;
	}
	slam_lua_event_start(runnable);
	return runnable;
}

void slam_runnable_delete(slam_runnable_t* runnable) {
	int fd;
	slam_lua_event_stop(runnable);
	slam_free(runnable->entryfile);
	slam_lua_closestate(runnable->L);
	slam_poll_delete(runnable->poll);
	for (fd = 0; fd < MAX_SOCKET_NUMBER; ++fd) {
		if (runnable->sockets[fd] != nullptr) {
			slam_socket_delete(runnable->sockets[fd]);
		}
	}
	slam_free(runnable);
}

void slam_runnable_accept_socket(slam_runnable_t* runnable, slam_socket_t* socket) {
	assert(socket->type == SOCKET_SERVER);
	while (true) {
		slam_socket_t* newsocket = slam_socket_accept(socket);
		if (!newsocket) {
			return; /* EAGAIN */
		}
		assert(newsocket->fd < MAX_SOCKET_NUMBER);
		assert(runnable->sockets[newsocket->fd] == nullptr);
		if (slam_poll_add_socket(runnable->poll, newsocket->fd)) {
			runnable->sockets[newsocket->fd] = newsocket;
			slam_lua_event_establish_connection(runnable, newsocket->fd);
		}
		else {
			slam_socket_delete(newsocket);
		}
	}
}

void slam_runnable_read_socket(slam_runnable_t* runnable, SOCKET fd) {
	slam_socket_t* socket = nullptr;
	assert(fd < MAX_SOCKET_NUMBER);
	if (runnable->sockets[fd] == nullptr) {
		return; /* already close */
	}
	socket = runnable->sockets[fd];
	if (socket->type == SOCKET_SERVER) {
		slam_runnable_accept_socket(runnable, socket);
	}
	else {
		/* TODO: notify to lua */
		if (slam_socket_read(socket) < 0 || !slam_protocol_decode(runnable, socket)) {
			slam_runnable_remove_socket(runnable, fd);
		}
	}
}

void slam_runnable_write_socket(slam_runnable_t* runnable, SOCKET fd) {
	slam_socket_t* socket = nullptr;
	assert(fd < MAX_SOCKET_NUMBER);
	if (runnable->sockets[fd] == nullptr) {
		return; /* already close */
	}
	socket = runnable->sockets[fd];
	if (socket->type != SOCKET_SERVER) {
		/* TODO: notify to lua */
		if (slam_socket_write_buffer(socket) < 0 || !slam_protocol_encode(runnable, socket)) {
			slam_runnable_remove_socket(runnable, fd);
		}
	}
}

void slam_runnable_do_message(slam_runnable_t* runnable) {
	slam_poll_event events[128];
	ssize_t i, events_number = slam_poll_wait(runnable->poll, events, 128, 0);
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
	slam_runnable_do_message(runnable);
}

void slam_runnable_remove_socket(slam_runnable_t* runnable, SOCKET fd) {
	assert(fd < MAX_SOCKET_NUMBER);
	if (runnable->sockets[fd] == nullptr) {
		return;
	}
	slam_poll_add_socket(runnable->poll, fd);
	slam_socket_delete(runnable->sockets[fd]);
	runnable->sockets[fd] = nullptr;
	slam_lua_event_lost_connection(runnable, fd);
}

slam_socket_t* slam_runnable_newserver(slam_runnable_t* runnable, const char* address, int port) {
	return nullptr;
}

slam_socket_t* slam_runnable_newclient(slam_runnable_t* runnable, const char* address, int port, int timeout) {
	return nullptr;
}

lua_State* slam_runnable_luastate(slam_runnable_t* runnable) {
	return runnable->L;
}


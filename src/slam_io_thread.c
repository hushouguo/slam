/*
 * \file: slam_io_thread.c
 * \brief: Created by hushouguo at 09:38:24 Aug 04 2019
 */

#include "slam.h"

#define MAX_SOCKET_NUMBER		65536

#define VALID_SOCKET(fd)		((fd) >= 0 && (fd) < MAX_SOCKET_NUMBER)
#define ASSERT_SOCKET(fd)		assert(VALID_SOCKET(fd))

struct slam_io_thread_s {
    pthread_t thread_handle;
    slam_message_queue_t* mq;
	slam_poll_t* poll;
	slam_socket_t* sockets[MAX_SOCKET_NUMBER];    
};

void slam_io_thread_accept_socket(slam_io_t* io, slam_socket_t* socket) {
	assert(socket->type == SOCKET_SERVER);
	while (true) {
		slam_socket_t* newsocket = slam_socket_accept(socket);
		if (!newsocket) {
			return; /* EAGAIN */
		}
		if (!slam_io_add_socket(io, newsocket)) {
			slam_socket_delete(newsocket);
		}
	}
}

void slam_io_thread_read_socket(slam_io_t* io, SOCKET fd) {
	slam_socket_t* socket = nullptr;
	ASSERT_SOCKET(fd);
	if (io->sockets[fd] == nullptr) {
		return; /* already close */
	}
	socket = io->sockets[fd];
	if (socket->type == SOCKET_SERVER) {
		slam_io_accept_socket(io, socket);
	}
	else {		
		if (slam_socket_read(socket) < 0 || !slam_protocol_decode(io, socket)) {
			slam_io_remove_socket(io, fd);
		}
	}
}

void slam_io_thread_write_socket(slam_io_thread_t* io, SOCKET fd) {
	ASSERT_SOCKET(fd);
	if (io->sockets[fd] == nullptr) {
		return; /* already close */
	}
	slam_socket_t* socket = io->sockets[fd];
	if (socket->type != SOCKET_SERVER) {
		if (slam_socket_write_buffer(socket) < 0) {
			slam_io_thread_remove_socket(io, fd);
		}
	}
}

void slam_io_thread_do_message(slam_io_thread_t* io, int timeout) {
	slam_poll_event events[128];
	ssize_t i, events_number = slam_poll_wait(io->poll, events, 128, timeout);
	for (i = 0; i < events_number; ++i) {
		slam_poll_event* event = &events[i];
		if (event->events & EPOLLERR) {
			CHECK_RETURN(false, (void)0, "poll error: %d,%s", errno, strerror(errno));
			slam_io_thread_remove_socket(io, event->data.fd);
		}
		else if (event->events & EPOLLHUP) {
			CHECK_RETURN(false, (void)0, "poll hup: %d,%s", errno, strerror(errno));
			slam_io_thread_remove_socket(io, event->data.fd);
		}
		else if (event->events & EPOLLRDHUP) {
			CHECK_RETURN(false, (void)0, "poll rdhup: %d,%s", errno, strerror(errno));
			slam_io_thread_remove_socket(io, event->data.fd);
		}
		else {
			if (event->events & EPOLLIN) {
				slam_io_thread_read_socket(io, event->data.fd);
			}
			if (event->events & EPOLLOUT) {
				slam_io_thread_write_socket(io, event->data.fd);
			}
		}
	}
}

void slam_io_thread_run(slam_io_thread_t* io) {
    while (true) {
        slam_io_thread_do_message(io, -1);
    }
}

slam_io_thread_t* slam_io_thread_new(slam_message_queue_t* mq) {
    slam_io_thread_t* io = (slam_io_thread_t *) slam_malloc(sizeof(slam_io_thread_t));
    io->mq = mq;
	if (pthread_create(&io->thread_handle, nullptr, &slam_io_thread_run, io)) {
		slam_free(io);
	}
	io->poll = slam_poll_new();
	memset(io->sockets, 0, sizeof(io->sockets));
	
	return io;
}

void slam_io_thread_delete(slam_io_thread_t* io) {
    int fd;
    pthread_join(io->thread_handle, nullptr);
	slam_poll_delete(io->poll);
	for (fd = 0; fd < MAX_SOCKET_NUMBER; ++fd) {
		if (io->sockets[fd] != nullptr) {
			slam_socket_delete(io->sockets[fd]);
		}
	}
    slam_free(io);
}

// extern interface
extern SOCKET slam_io_newserver(slam_io_thread_t* io, const char* address, int port);
extern SOCKET slam_io_newclient(slam_io_thread_t* io, const char* address, int port, int timeout);
extern void slam_io_closesocket(slam_io_thread_t* io, SOCKET fd);
extern bool slam_io_response(slam_io_thread_t* io, slam_message_t* message);

slam_socket_t* slam_io_newserver(slam_io_t* io, const char* address, int port) {
	slam_socket_t* socket = slam_socket_new();
	bool rc = slam_socket_bind(socket, address, port);
	if (!rc) {
		slam_socket_delete(socket);
		return nullptr;
	}
	if (!slam_io_add_socket(io, socket)) {
		slam_socket_delete(socket);
	}	
	return socket;
}

slam_socket_t* slam_io_newclient(slam_io_t* io, const char* address, int port, int timeout) {
	slam_socket_t* socket = slam_socket_new();
	bool rc = slam_socket_connect(socket, address, port, timeout);
	if (!rc) {
		slam_socket_delete(socket);
		return nullptr;
	}
	if (!slam_io_add_socket(io, socket)) {
		slam_socket_delete(socket);
	}	
	return socket;
}

slam_socket_t* slam_io_socket(slam_io_t* io, SOCKET fd) {
	return fd >= 0 && fd < MAX_SOCKET_NUMBER ? io->sockets[fd] : nullptr;
}



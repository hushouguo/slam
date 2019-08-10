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
	int pipefd[2];
};

bool slam_io_thread_establish_connection_event(slam_io_thread_t* io, SOCKET fd) {
    slam_message_t* message = slam_message_new(fd, MESSAGE_TYPE_ESTABLISH_CONNECTION, 0);
    if (!slam_message_queue_push_back(io->mq, message)) {
        slam_message_delete(message);
        return false; // io->mq is full
    }
    return true;
}

bool slam_io_thread_lost_connection_event(slam_io_thread_t* io, SOCKET fd) {
    slam_message_t* message = slam_message_new(fd, MESSAGE_TYPE_LOST_CONNECTION, 0);
    if (!slam_message_queue_push_back(io->mq, message)) {
        slam_message_delete(message);
        return false; // io->mq is full
    }
    return true;
}

bool slam_io_thread_add_socket(slam_io_thread_t* io, slam_socket_t* newsocket) {
	ASSERT_SOCKET(newsocket->fd);
	assert(io->sockets[newsocket->fd] == nullptr);
	if (!slam_poll_add_socket(io->poll, newsocket->fd)) {
		return false;
	}
	io->sockets[newsocket->fd] = newsocket;
	return true;
}

void slam_io_thread_remove_socket(slam_io_thread_t* io, SOCKET fd) {
	ASSERT_SOCKET(fd);
	if (io->sockets[fd] == nullptr) {
		return;
	}
	slam_poll_remove_socket(io->poll, fd);
	slam_socket_delete(io->sockets[fd]);
	io->sockets[fd] = nullptr;
	slam_io_thread_lost_connection_event(io, fd);
}

void slam_io_thread_accept_socket(slam_io_thread_t* io, slam_socket_t* socket) {
	assert(socket->type == SOCKET_SERVER);
	while (true) {
		slam_socket_t* newsocket = slam_socket_accept(socket);
		if (!newsocket) {
			return; /* EAGAIN */
		}
		if (!slam_io_thread_add_socket(io, newsocket)) {
			slam_socket_delete(newsocket);
		}
		else {
			slam_io_thread_establish_connection_event(io, newsocket->fd);
		}
	}
}

void slam_io_thread_read_socket(slam_io_thread_t* io, SOCKET fd) {
	ASSERT_SOCKET(fd);
	if (io->sockets[fd] == nullptr) {
		return; /* already close */
	}
	slam_socket_t* socket = io->sockets[fd];
	if (socket->type == SOCKET_SERVER) {
		slam_io_thread_accept_socket(io, socket);
	}
	else {		
		if (!slam_socket_poll_read(socket, io->mq)) {
			slam_io_thread_remove_socket(io, fd);
		}
	}
}

void slam_io_thread_write_socket(slam_io_thread_t* io, SOCKET fd) {
	ASSERT_SOCKET(fd);
	if (io->sockets[fd] == nullptr) {
		return; /* already close */
	}
	slam_socket_t* socket = io->sockets[fd];
	assert(socket->type != SOCKET_SERVER);
	if (!slam_socket_poll_write(socket)) {
		slam_io_thread_remove_socket(io, fd);
	}
}

void slam_io_thread_do_message(slam_io_thread_t* io, int timeout) {
	slam_poll_event events[__slam_main->poll_event_number];
	ssize_t i, events_number = slam_poll_wait(io->poll, events, __slam_main->poll_event_number, timeout);
	Debug("------------------- io thread wakeup --------------------");
	if (__slam_main->halt) {
	    return; // after waking up, check the exit mark first
	}
	for (i = 0; i < events_number; ++i) {
		slam_poll_event* event = &events[i];
		if (event->events & EPOLLERR) {
			slam_io_thread_remove_socket(io, event->data.fd);
			CHECK_CONTINUE(false, "poll error: %d,%s, fd: %d", errno, errstring(errno), event->data.fd);
		}
		else if (event->events & EPOLLHUP) {
			slam_io_thread_remove_socket(io, event->data.fd);
			CHECK_CONTINUE(false, "poll hup: %d,%s, fd: %d", errno, errstring(errno), event->data.fd);
		}
		else if (event->events & EPOLLRDHUP) {
			slam_io_thread_remove_socket(io, event->data.fd);
			CHECK_CONTINUE(false, "poll rdhup: %d,%s, fd: %d", errno, errstring(errno), event->data.fd);
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

void* slam_io_thread_run(void* arg) {
	slam_io_thread_t* io = (slam_io_thread_t *) arg;
    Debug("slam_io_thread start");
    while (!__slam_main->halt) {
        slam_io_thread_do_message(io, -1);
    }
    Debug("slam_io_thread stop");
	return nullptr;
}

slam_io_thread_t* slam_io_thread_new(slam_message_queue_t* mq) {
    slam_io_thread_t* io = (slam_io_thread_t *) slam_malloc(sizeof(slam_io_thread_t));
    io->mq = mq;
	memset(io->sockets, 0, sizeof(io->sockets));
	io->poll = slam_poll_new();
	if (pipe(io->pipefd) || !slam_poll_add_socket(io->poll, io->pipefd[0])) {
	    slam_poll_delete(io->poll);	slam_free(io);
		return nullptr;
	}
	if (pthread_create(&io->thread_handle, nullptr, &slam_io_thread_run, io)) {
		slam_poll_delete(io->poll); slam_free(io);
		return nullptr;
	}
	return io;
}

void slam_io_thread_delete(slam_io_thread_t* io) {
    int fd = 0;
    write(io->pipefd[1], &fd, sizeof(fd));    
    pthread_join(io->thread_handle, nullptr);
	slam_poll_delete(io->poll);
	for (fd = 0; fd < MAX_SOCKET_NUMBER; ++fd) {
		if (io->sockets[fd] != nullptr) {
			slam_socket_delete(io->sockets[fd]);
		}
	}
	slam_close(io->pipefd[0]);
	slam_close(io->pipefd[1]);
    slam_free(io);
}

// extern interface
SOCKET slam_io_newserver(slam_io_thread_t* io, const char* address, int port) {
	slam_socket_t* socket = slam_socket_new();
	if (!slam_socket_bind(socket, address, port)) {
		slam_socket_delete(socket);
		return INVALID_SOCKET;
	}
	if (!slam_io_thread_add_socket(io, socket)) {
		slam_socket_delete(socket);
		return INVALID_SOCKET;
	}	
	return socket->fd;
}

SOCKET slam_io_newclient(slam_io_thread_t* io, const char* address, int port, int timeout) {
	slam_socket_t* socket = slam_socket_new();
	if (!slam_socket_connect(socket, address, port, timeout)) {
		slam_socket_delete(socket);
		return INVALID_SOCKET;
	}
	if (!slam_io_thread_add_socket(io, socket)) {
		slam_socket_delete(socket);
		return INVALID_SOCKET;
	}	
	return socket->fd;
}

void slam_io_closesocket(slam_io_thread_t* io, SOCKET fd) {
    slam_shutdown(fd, SHUT_RD); // trigger poll event to destroy socket
}

bool slam_io_response(slam_io_thread_t* io, slam_message_t* message) {
    ASSERT_SOCKET(message->fd);
	if (io->sockets[message->fd] == nullptr) {
		return false; /* already close */
	}
	slam_socket_t* socket = io->sockets[message->fd];
	message->bytesize = 0; // reset bytesize
	if (!slam_socket_write_message(socket, message)) {
	    return false; // mq is full
	}
	return slam_poll_set_socket_pollout(io->poll, message->fd, true); // trigger poll out event for this fd
}


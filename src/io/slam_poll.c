/*
 * \file: slam_poll.c
 * \brief: Created by hushouguo at 16:14:08 Jul 23 2019
 */

#include "slam.h"

struct slam_poll_s {
	 int epfd;
};

slam_poll_t* slam_poll_new() {
	slam_poll_t* poll = (slam_poll_t *) slam_malloc(sizeof(slam_poll_t));
	poll->epfd = epoll_create(128);
	return poll;
}

void slam_poll_delete(slam_poll_t* poll) {
	slam_close(poll->epfd);
	slam_free(poll);
}

bool slam_poll_add_socket(slam_poll_t* poll, SOCKET fd) {
	struct epoll_event event;
	//event.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR;
	event.events = EPOLLET | EPOLLIN | EPOLLERR;
	event.data.u64 = 0; /* avoid valgrind warning */
	event.data.fd = fd;
	int rc = epoll_ctl(poll->epfd, EPOLL_CTL_ADD, fd, &event); 	
	CHECK_RETURN(rc == 0, false, "epoll_add error: %d, %s", errno, errstring(errno));
	return true;
}

bool slam_poll_remove_socket(slam_poll_t* poll, SOCKET fd) {
	struct epoll_event event;
	event.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR;
	event.data.u64 = 0; /* avoid valgrind warning */
	event.data.fd = fd;
	/* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
	int rc = epoll_ctl(poll->epfd, EPOLL_CTL_DEL, fd, &event);
	CHECK_RETURN(rc == 0, false, "epoll_del error: %d, %s", errno, errstring(errno));
	return true;
}

bool slam_poll_set_socket_pollout(slam_poll_t* poll, SOCKET fd, bool pollout) {
    struct epoll_event event;
    event.events = EPOLLET | EPOLLIN | EPOLLERR;
    if (pollout) {
        event.events |= EPOLLOUT;
    }
    event.data.u64 = 0; /* avoid valgrind warning */
    event.data.fd = fd;
    /* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
    int rc = epoll_ctl(poll->epfd, EPOLL_CTL_MOD, fd, &event);
    CHECK_RETURN(rc == 0, false, "epoll_mod error: %d, %s", errno, errstring(errno));
    return true;
}

ssize_t slam_poll_wait(slam_poll_t* poll, slam_poll_event* events, size_t events_size, int timeout) {
	int events_number = TEMP_FAILURE_RETRY(
	                        epoll_wait(
	                            poll->epfd, 
	                            events, 
	                            events_size, 
    // timeout: -1 to block indefinitely, 0 to return immediately, even if no events are available	                            
	                            timeout 
	                        ));
	if (events_number < 0) {
		if (errno == EINTR) {
			return 0; /* wake up by signal */
		}
		CHECK_RETURN(false, -1, "epoll wait error: %d, %s", errno, errstring(errno));
	}
	return events_number;
}
 

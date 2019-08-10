/*
 * \file: slam_poll.h
 * \brief: Created by hushouguo at 16:12:17 Jul 23 2019
 */
 
#ifndef __SLAM_POLL_H__
#define __SLAM_POLL_H__

typedef struct slam_poll_s slam_poll_t;
typedef struct epoll_event slam_poll_event;

extern slam_poll_t* slam_poll_new();
extern void slam_poll_delete(slam_poll_t* poll);

extern bool slam_poll_add_socket(slam_poll_t* poll, SOCKET fd);
extern bool slam_poll_remove_socket(slam_poll_t* poll, SOCKET fd);
extern bool slam_poll_set_socket_pollout(slam_poll_t* poll, SOCKET fd, bool pollout);

extern ssize_t slam_poll_wait(slam_poll_t* poll, slam_poll_event* events, size_t events_size, int timeout);

#endif

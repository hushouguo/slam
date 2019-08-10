/*
 * \file: slam_socket.h
 * \brief: Created by hushouguo at 15:47:20 Jul 23 2019
 */
 
#ifndef __SLAM_SOCKET_H__
#define __SLAM_SOCKET_H__

enum { // SocketType
	SOCKET_NONE = 0, SOCKET_SERVER, SOCKET_CLIENT, SOCKET_CONNECTION
};

struct slam_socket_s {
	 SOCKET fd;
	 int type;
	 slam_message_t* readmessage;
	 slam_message_queue_t* sendmq;
};

typedef struct slam_socket_s slam_socket_t;

extern slam_socket_t* slam_socket_new();
extern void slam_socket_delete(slam_socket_t* socket);

extern bool slam_socket_bind(slam_socket_t* socket, const char* address, int port);
extern bool slam_socket_connect(slam_socket_t* socket, const char* address, int port, int timeout);

extern slam_socket_t* slam_socket_accept(slam_socket_t* socket);
extern bool slam_socket_poll_read(slam_socket_t* socket, slam_message_queue_t* readmq);
extern bool slam_socket_poll_write(slam_socket_t* socket);
extern bool slam_socket_write_message(slam_socket_t* socket, const slam_message_t* message);

#endif


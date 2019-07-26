/*
 * \file: slam_socket.h
 * \brief: Created by hushouguo at 15:47:20 Jul 23 2019
 */
 
#ifndef __SLAM_SOCKET_H__
#define __SLAM_SOCKET_H__

#ifdef PLATFORM_WINDOWS
#define SOCKET						intptr_t
#else
#define SOCKET						int
#endif

#define INVALID_SOCKET				-1

enum {
	SOCKET_NONE = 0, SOCKET_SERVER, SOCKET_CLIENT, SOCKET_CONNECTION
};

struct slam_socket_s {
	 SOCKET fd;
	 int type;
	 slam_byte_buffer_t* rbuffer;
	 slam_byte_buffer_t* wbuffer;
};

typedef struct slam_socket_s slam_socket_t;

extern slam_socket_t* slam_socket_new();
extern void slam_socket_delete(slam_socket_t* socket);

extern bool slam_socket_bind(slam_socket_t* socket, const char* address, int port);
extern bool slam_socket_connect(slam_socket_t* socket, const char* address, int port, int timeout);

extern slam_socket_t* slam_socket_accept(slam_socket_t* socket);

extern ssize_t slam_socket_read(slam_socket_t* socket);
extern ssize_t slam_socket_write(slam_socket_t* socket, const void* buf, size_t bufsize);
extern ssize_t slam_socket_write_buffer(slam_socket_t* socket);

#endif


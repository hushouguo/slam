/*
 * \file: slam_io_thread.h
 * \brief: Created by hushouguo at 09:38:21 Aug 04 2019
 */
 
#ifndef __SLAM_IO_THREAD_H__
#define __SLAM_IO_THREAD_H__

typedef struct slam_io_thread_s slam_io_thread_t;

extern slam_io_thread_t* slam_io_thread_new(slam_message_queue_t* mq);
extern void slam_io_thread_delete(slam_io_thread_t* io);

// extern interface
extern SOCKET slam_io_newserver(slam_io_thread_t* io, const char* address, int port);
extern SOCKET slam_io_newclient(slam_io_thread_t* io, const char* address, int port, int timeout);
extern void slam_io_closesocket(slam_io_thread_t* io, SOCKET fd);
extern bool slam_io_response(slam_io_thread_t* io, slam_message_t* message);
    
#endif

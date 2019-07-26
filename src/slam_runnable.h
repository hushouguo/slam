/*
 * \file: slam_runnable.h
 * \brief: Created by hushouguo at 16:39:10 Jul 25 2019
 */
 
#ifndef __SLAM_RUNNABLE_H__
#define __SLAM_RUNNABLE_H__

typedef struct slam_runnable_s slam_runnable_t;

extern slam_runnable_t* slam_runnable_new(const char* entryfile);
extern void slam_runnable_delete(slam_runnable_t* runnable);

extern void slam_runnable_run(slam_runnable_t* runnable);

extern void slam_runnable_remove_socket(slam_runnable_t* runnable, SOCKET fd);
extern slam_socket_t* slam_runnable_newserver(slam_runnable_t* runnable, const char* address, int port);
extern slam_socket_t* slam_runnable_newclient(slam_runnable_t* runnable, const char* address, int port, int timeout);

extern lua_State* slam_runnable_luastate(slam_runnable_t* runnable);

#endif


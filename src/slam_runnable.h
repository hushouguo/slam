/*
 * \file: slam_runnable.h
 * \brief: Created by hushouguo at 16:39:10 Jul 25 2019
 */
 
#ifndef __SLAM_RUNNABLE_H__
#define __SLAM_RUNNABLE_H__

typedef struct slam_runnable_s slam_runnable_t;

extern slam_runnable_t* slam_runnable_new();
extern void slam_runnable_delete(slam_runnable_t* runnable);

extern void slam_runnable_run(slam_runnable_t* runnable);

extern bool slam_runnable_load_entryfile(slam_runnable_t* runnable, const char* entryfile);
extern bool slam_runnable_load_protocol(slam_runnable_t* runnable, const char* dynamic_lib_name);
extern bool slam_runnable_reload_entryfile(slam_runnable_t* runnable, const char* entryfile);
extern bool slam_runnable_reload_protocol(slam_runnable_t* runnable, const char* dynamic_lib_name);

extern void slam_runnable_remove_socket(slam_runnable_t* runnable, SOCKET fd);
extern void slam_runnable_write_socket(slam_runnable_t* runnable, SOCKET fd);
extern slam_socket_t* slam_runnable_newserver(slam_runnable_t* runnable, const char* address, int port);
extern slam_socket_t* slam_runnable_newclient(slam_runnable_t* runnable, const char* address, int port, int timeout);

extern slam_lua_t* slam_runnable_lua(slam_runnable_t* runnable);
extern slam_protocol_t* slam_runnable_protocol(slam_runnable_t* runnable);
extern slam_socket_t* slam_runnable_socket(slam_runnable_t* runnable, SOCKET fd);
extern slam_timer_list_t* slam_runnable_timer_list(slam_runnable_t* runnable);

extern slam_timer_t* slam_runnable_add_timer(slam_runnable_t* runnable, uint64_t interval, bool forever, int ref, slam_lua_value_t* ctx);
extern void slam_runnable_remove_timer(slam_runnable_t* runnable, uint32_t timerid);

#endif


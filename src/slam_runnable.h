/*
 * \file: slam_runnable.h
 * \brief: Created by hushouguo at 16:39:10 Jul 25 2019
 */
 
#ifndef __SLAM_RUNNABLE_H__
#define __SLAM_RUNNABLE_H__


struct slam_runnable_s {
	slam_poll_t* poll;
	slam_socket_t* sockets[MAX_SOCKET_NUMBER];
	slam_protocol_t* protocol;
	slam_lua_t* lua;
	slam_timer_list_t* timer_list;
	slam_message_t* message;
};

typedef struct slam_runnable_s slam_runnable_t;

extern slam_runnable_t* slam_runnable_new();
extern void slam_runnable_delete(slam_runnable_t* runnable);

extern void slam_runnable_run(slam_runnable_t* runnable);

// load & reload entryfile and protocol lib
extern bool slam_runnable_load_entryfile(slam_runnable_t* runnable, const char* entryfile);
extern bool slam_runnable_load_protocol(slam_runnable_t* runnable, const char* dynamic_lib_name);

// network interface
extern slam_socket_t* slam_runnable_socket(slam_runnable_t* runnable, SOCKET fd);
extern void slam_runnable_remove_socket(slam_runnable_t* runnable, SOCKET fd);
extern void slam_runnable_write_socket(slam_runnable_t* runnable, SOCKET fd);
extern slam_socket_t* slam_runnable_newserver(slam_runnable_t* runnable, const char* address, int port);
extern slam_socket_t* slam_runnable_newclient(slam_runnable_t* runnable, const char* address, int port, int timeout);

// add & remove timer
extern slam_timer_t* slam_runnable_add_timer(slam_runnable_t* runnable, uint64_t interval, bool forever, int ref, slam_lua_value_t* ctx);
extern void slam_runnable_remove_timer(slam_runnable_t* runnable, int ref);

// registe message
extern const char* slam_runnable_reg_message(slam_runnable_t* runnable, msgid_t msgid, const char* typename);

#endif


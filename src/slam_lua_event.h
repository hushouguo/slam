/*
 * \file: slam_lua_event.h
 * \brief: Created by hushouguo at 15:48:04 Jul 23 2019
 */
 
#ifndef __SLAM_LUA_EVENT_H__
#define __SLAM_LUA_EVENT_H__

extern bool slam_lua_event_start(slam_runnable_t* runnable);
extern bool slam_lua_event_stop(slam_runnable_t* runnable);
extern bool slam_lua_event_establish_connection(slam_runnable_t* runnable, SOCKET fd);
extern bool slam_lua_event_lost_connection(slam_runnable_t* runnable, SOCKET fd);
extern bool slam_lua_event_message(slam_runnable_t* runnable, SOCKET fd, msgid_t msgid);
extern bool slam_lua_event_timer(slam_runnable_t* runnable, slam_timer_t* timer);

#endif

/*
 * \file: slam_protocol.h
 * \brief: Created by hushouguo at 10:42:00 Jul 26 2019
 */
 
#ifndef __SLAM_PROTOCOL_H__
#define __SLAM_PROTOCOL_H__

typedef struct slam_protocol_s slam_protocol_t;
typedef struct slam_runnable_s slam_runnable_t;

extern slam_protocol_t* slam_protocol_new();
extern void slam_protocol_delete(slam_protocol_t* protocol);

extern bool slam_protocol_load_dynamic_lib(slam_protocol_t* protocol, const char* dynamic_lib);
extern bool slam_protocol_reload_dynamic_lib(slam_protocol_t* protocol, const char* dynamic_lib);

/* load proto descriptor */
extern bool slam_protocol_load_descriptor(slam_runnable_t* runnable, const char* filename);

/* registe message */
extern bool slam_protocol_reg_message(slam_runnable_t* runnable, msgid_t msgid, const char* name);

/* decode socket->rbuffer to lua's table */
extern bool slam_protocol_decode(slam_runnable_t* runnable, const slam_socket_t* socket);

/* encode lua's table to socket->wbuffer */
extern bool slam_protocol_encode(slam_runnable_t* runnable, msgid_t msgid, slam_socket_t* socket);

#endif

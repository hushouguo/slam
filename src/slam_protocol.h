/*
 * \file: slam_protocol.h
 * \brief: Created by hushouguo at 10:42:00 Jul 26 2019
 */
 
#ifndef __SLAM_PROTOCOL_H__
#define __SLAM_PROTOCOL_H__

typedef struct slam_runnable_s slam_runnable_t;

#define MAX_MESSAGE_ID	65536

struct slam_protocol_s {
    const char* regtable[MAX_MESSAGE_ID];   // msgid => registry value
};

typedef struct slam_protocol_s slam_protocol_t;

extern slam_protocol_t* slam_protocol_new();
extern void slam_protocol_delete(slam_protocol_t* protocol);

/* load proto descriptor */
extern bool slam_protocol_load_descriptor(slam_protocol_t* protocol, const char* filename);

/* registe message */
extern const char* slam_protocol_reg_message(slam_protocol_t* protocol, msgid_t msgid, const char* typename);

/* msgid => registry value */
extern const char* slam_protocol_find_name(slam_protocol_t* protocol, msgid_t msgid);

/* decode buffer to lua's table */
extern bool slam_protocol_decode(slam_runnable_t* runnable, const slam_message_t* message);

/* encode lua's table to buffer */
extern slam_message_t* slam_protocol_encode(slam_runnable_t* runnable, SOCKET fd, msgid_t msgid);

#endif

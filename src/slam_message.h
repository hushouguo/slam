/*
 * \file: slam_message.h
 * \brief: Created by hushouguo at 10:46:29 Aug 02 2019
 */
 
#ifndef __SLAM_MESSAGE_H__
#define __SLAM_MESSAGE_H__

#define MESSAGE_FLAG_ESTABLISH_CONNECTION       1
#define MESSAGE_FLAG_LOST_CONNECTION            2
#define MESSAGE_FLAG_PROTOBUF_PACKAGE           4

struct slam_message_s {
    SOCKET fd;
    int flags;
    msgid_t msgid;
    size_t bufsize, bytesize;
    byte_t buf[0];
};

typedef struct slam_message_s slam_message_t;

extern slam_message_t* slam_message_new(SOCKET, int, msgid_t, size_t);
extern void slam_message_delete(slam_message_t* message);

//extern const char* slam_message_reg(slam_message_t* message, msgid_t msgid, const char* typename);
//extern const char* slam_message_find(slam_message_t* message, msgid_t msgid);

#endif

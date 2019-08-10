/*
 * \file: slam_message.h
 * \brief: Created by hushouguo at 10:46:29 Aug 02 2019
 */
 
#ifndef __SLAM_MESSAGE_H__
#define __SLAM_MESSAGE_H__

enum {
    MESSAGE_TYPE_ESTABLISH_CONNECTION = 1,
    MESSAGE_TYPE_LOST_CONNECTION,
    MESSAGE_TYPE_PROTOBUF_PACKAGE
};

#define PACKAGE_MIN_SIZE                        sizeof(slam_rawmessage_t)
#define PACKAGE_MAX_SIZE                        __slam_main->max_package_size

#pragma pack(push, 1)
    struct slam_rawmessage_s {
    	msglen_t len;
    	entityid_t entityid;
    	msgid_t msgid;
    	uint16_t flags;
    	char payload[0];
    };
#pragma pack(pop)

typedef struct slam_rawmessage_s slam_rawmessage_t;

struct slam_message_s {
    SOCKET fd;
    int type;
    size_t bufsize, bytesize;
    byte_t buf[0];
};

typedef struct slam_message_s slam_message_t;

extern slam_message_t* slam_message_new(SOCKET fd, int type, size_t bufsize);
extern void slam_message_delete(slam_message_t* message);

#endif

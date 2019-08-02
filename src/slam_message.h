/*
 * \file: slam_message.h
 * \brief: Created by hushouguo at 10:46:29 Aug 02 2019
 */
 
#ifndef __SLAM_MESSAGE_H__
#define __SLAM_MESSAGE_H__

typedef struct slam_message_s slam_message_t;

extern slam_message_t* slam_message_new();
extern void slam_message_delete(slam_message_t* message);

extern const char* slam_message_reg(slam_message_t* message, msgid_t msgid, const char* typename);
extern const char* slam_message_find(slam_message_t* message, msgid_t msgid);

#endif

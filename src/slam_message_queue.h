/*
 * \file: slam_message_queue.h
 * \brief: Created by hushouguo at 09:38:59 Aug 04 2019
 */
 
#ifndef __SLAM_MESSAGE_QUEUE_H__
#define __SLAM_MESSAGE_QUEUE_H__

typedef struct slam_message_queue_s slam_message_queue_t;

extern slam_message_queue_t* slam_message_queue_new();
extern void slam_message_queue_delete(slam_message_queue_t* mq);

// return false means mq is full
extern bool slam_message_queue_push_back(slam_message_queue_t* mq, slam_message_t* msg);
extern bool slam_message_queue_empty(slam_message_queue_t* mq);
extern slam_message_t* slam_message_queue_pop_front(slam_message_queue_t* mq);

#endif

/*
 * \file: slam_message_queue.c
 * \brief: Created by hushouguo at 09:39:01 Aug 04 2019
 */

#include "slam.h"

#define MESSAGE_QUEUE_SIZE      10000

// ring buffer
struct struct slam_message_queue_s {
    slam_message_t* queue[MESSAGE_QUEUE_SIZE];
    uint32_t rindex, windex;
};

slam_message_queue_t* slam_message_queue_new() {
    slam_message_queue_t* mq = (slam_message_queue_t *) slam_malloc(sizeof(slam_message_queue_t));
    mq->rindex = mq->windex = 0;
    return mq;
}

void slam_message_queue_delete(slam_message_queue_t* mq) {
    //TODO: slam_free(message);
}


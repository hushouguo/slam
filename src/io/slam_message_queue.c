/*
 * \file: slam_message_queue.c
 * \brief: Created by hushouguo at 09:39:01 Aug 04 2019
 */

#include "slam.h"

//#define MESSAGE_QUEUE_SIZE      __slam_main->message_queue_size
#define MESSAGE_QUEUE_SIZE      10000

// ring buffer
struct slam_message_queue_s {
    slam_message_t* queue[MESSAGE_QUEUE_SIZE];
    uint32_t rindex, windex;
};

slam_message_queue_t* slam_message_queue_new() {
    slam_message_queue_t* mq = (slam_message_queue_t *) slam_malloc(sizeof(slam_message_queue_t));
    mq->rindex = mq->windex = 0;
    return mq;
}

void slam_message_queue_delete(slam_message_queue_t* mq) {
    while (!slam_message_queue_empty(mq)) {
        slam_message_t* message = slam_message_queue_pop_front(mq);
        slam_message_delete(message);
    }
}

// return false means mq is full
bool slam_message_queue_push_back(slam_message_queue_t* mq, const slam_message_t* message) {
    assert(mq->rindex >= 0 && mq->rindex < MESSAGE_QUEUE_SIZE);
    assert(mq->windex >= 0 && mq->windex < MESSAGE_QUEUE_SIZE);
    uint32_t newindex = (mq->windex + 1) % MESSAGE_QUEUE_SIZE;
    CHECK_RETURN(newindex != mq->rindex, false, "mq is full");
    mq->queue[mq->windex] = (slam_message_t *) message;
    mq->windex = newindex;
    //TODO: wait cond
    return true;
}

bool slam_message_queue_empty(slam_message_queue_t* mq) {
    return mq->rindex == mq->windex;
}

slam_message_t* slam_message_queue_pop_front(slam_message_queue_t* mq) {
    CHECK_RETURN(mq->rindex != mq->windex, nullptr, "mq is empty");
    return mq->queue[mq->rindex++];
}

slam_message_t* slam_message_queue_front(slam_message_queue_t* mq) {
    CHECK_RETURN(mq->rindex != mq->windex, nullptr, "mq is empty");
    return mq->queue[mq->rindex];
}


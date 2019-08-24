/*
 * \file: slam_message_queue.c
 * \brief: Created by hushouguo at 09:39:01 Aug 04 2019
 */

#include "slam.h"

#define ASSERT_QUEUE(mq)    \
    do {\
        assert(mq->rindex >= 0 && mq->rindex < mq->maxsize);\
        assert(mq->windex >= 0 && mq->windex < mq->maxsize);\
    } while (false)


slam_message_queue_t* slam_message_queue_new(size_t maxsize) {
    assert(maxsize > 0);
    slam_message_queue_t* mq = (slam_message_queue_t *) slam_malloc(sizeof(slam_message_queue_t));
    mq->queue = (slam_message_t **) slam_malloc(maxsize * sizeof(slam_message_t *));
    mq->maxsize = maxsize;
    mq->rindex = mq->windex = 0;
    mq->read_number = mq->write_number = 0;
    return mq;
}

void slam_message_queue_delete(slam_message_queue_t* mq) {
    while (!slam_message_queue_empty(mq)) {
        slam_message_t* message = slam_message_queue_pop_front(mq);
        slam_message_delete(message);
    }
    slam_free(mq->queue);
    slam_free(mq);
}

// return false means mq is full
bool slam_message_queue_push_back(slam_message_queue_t* mq, const slam_message_t* message) {
    ASSERT_QUEUE(mq);
    uint32_t newindex = (mq->windex + 1) % mq->maxsize;
    CHECK_RETURN(newindex != mq->rindex, false, "mq is full");
    mq->queue[mq->windex] = (slam_message_t *) message;
    mq->windex = newindex;
    ++ mq->write_number;
    return true;
}

bool slam_message_queue_empty(slam_message_queue_t* mq) {
    ASSERT_QUEUE(mq);
    return mq->rindex == mq->windex;
}

slam_message_t* slam_message_queue_pop_front(slam_message_queue_t* mq) {
    ASSERT_QUEUE(mq);
    CHECK_RETURN(mq->rindex != mq->windex, nullptr, "mq is empty");
    ++ mq->read_number;
    return mq->queue[mq->rindex++];
}

slam_message_t* slam_message_queue_front(slam_message_queue_t* mq) {
    ASSERT_QUEUE(mq);
    CHECK_RETURN(mq->rindex != mq->windex, nullptr, "mq is empty");
    return mq->queue[mq->rindex];
}

uint32_t slam_message_queue_number(slam_message_queue_t* mq) {
    ASSERT_QUEUE(mq);
    assert(mq->write_number >= mq->read_number);
    return mq->write_number - mq->read_number;
}


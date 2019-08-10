/*
 * \file: slam_thread_cond.c
 * \brief: Created by hushouguo at 09:41:06 Aug 04 2019
 */

#include "slam.h"

slam_thread_cond_t* slam_thread_cond_new() {
    slam_thread_cond_t* slam_cond = (slam_thread_cond_t *) slam_malloc(sizeof(slam_thread_cond_t));
    int rc = pthread_mutex_init(&slam_cond->mtx, nullptr);
    if (rc) {
        slam_free(slam_cond);
        CHECK_RETURN(false, nullptr, "pthread_mutex_init error: %d, %s", errno, errstring(errno));
    }
    rc = pthread_cond_init(&slam_cond->cond, nullptr);
    if (rc) {
        slam_free(slam_cond);
        CHECK_RETURN(false, nullptr, "pthread_cond_init error: %d, %s", errno, errstring(errno));
    }
    return slam_cond;
}

void slam_thread_cond_delete(slam_thread_cond_t* slam_cond) {
    pthread_mutex_destroy(&slam_cond->mtx);
    pthread_cond_destroy(&slam_cond->cond);
    slam_free(slam_cond);
}

bool slam_thread_cond_signal(slam_thread_cond_t* slam_cond) {
    int rc = pthread_cond_signal(&slam_cond->cond);
    CHECK_RETURN(rc == 0, false, "pthread_cond_signal error: %d, %s", errno, errstring(errno));
    return true;
}

bool slam_thread_cond_wait(slam_thread_cond_t* slam_cond) {
    int rc = pthread_cond_wait(&slam_cond->cond, &slam_cond->mtx);
    CHECK_RETURN(rc == 0, false, "pthread_cond_wait error: %d, %s", errno, errstring(errno));
    return true;
}

bool slam_thread_cond_timewait(slam_thread_cond_t* slam_cond, int64_t milliseconds) {
    if (milliseconds > 0) {
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
        int rc = pthread_cond_timedwait(&slam_cond->cond, &slam_cond->mtx, &ts);
        CHECK_RETURN(rc == 0, false, "pthread_cond_timedwait error: %d, %s", errno, errstring(errno));
        return true;
    }
    return slam_thread_cond_wait(slam_cond);
}

bool slam_thread_mutex_lock(pthread_mutex_t* mtx) {
    int rc = pthread_mutex_lock(mtx);
    CHECK_RETURN(rc == 0, false, "pthread_mutex_lock error: %d, %s", errno, errstring(errno));
    return true;
}

bool slam_thread_mutex_unlock(pthread_mutex_t* mtx) {
    int rc = pthread_mutex_unlock(mtx);
    CHECK_RETURN(rc == 0, false, "pthread_mutex_unlock error: %d, %s", errno, errstring(errno));
    return true;
}


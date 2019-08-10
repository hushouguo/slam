/*
 * \file: slam_thread_cond.h
 * \brief: Created by hushouguo at 09:41:04 Aug 04 2019
 */
 
#ifndef __SLAM_THREAD_COND_H__
#define __SLAM_THREAD_COND_H__

struct slam_thread_cond_s {
    pthread_mutex_t mtx;
    pthread_cond_t cond;
};

typedef struct slam_thread_cond_s slam_thread_cond_t;

extern slam_thread_cond_t* slam_thread_cond_new();
extern void slam_thread_cond_delete(slam_thread_cond_t* slam_cond);

extern bool slam_thread_cond_signal(slam_thread_cond_t* slam_cond);
extern bool slam_thread_cond_wait(slam_thread_cond_t* slam_cond);
extern bool slam_thread_cond_timewait(slam_thread_cond_t* slam_cond, int64_t milliseconds);

extern bool slam_thread_mutex_lock(pthread_mutex_t* mtx);
extern bool slam_thread_mutex_unlock(pthread_mutex_t* mtx);

#endif

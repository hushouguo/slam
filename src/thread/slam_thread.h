/*
 * \file: slam_thread.h
 * \brief: Created by hushouguo at 17:06:42 Jul 29 2019
 */
 
#ifndef __SLAM_THREAD_H__
#define __SLAM_THREAD_H__

typedef struct slam_thread_s slam_thread_t;

extern slam_thread_t* slam_thread_new(size_t id);
extern void slam_thread_delete(slam_thread_t* thread);

extern void slam_thread_join(slam_thread_t* thread);
extern void slam_thread_cond_wait(slam_thread_t* thread);
extern void slam_thread_cond_timewait(slam_thread_t* thread);

#endif

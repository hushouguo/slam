/*
 * \file: slam_thread_pool.h
 * \brief: Created by hushouguo at 17:12:05 Jul 29 2019
 */
 
#ifndef __SLAM_THREAD_POOL_H__
#define __SLAM_THREAD_POOL_H__

typedef struct slam_thread_pool_s slam_thread_pool_t;

extern slam_thread_pool_t* slam_thread_pool_new(); /* n: auto allocate */
extern void slam_thread_pool_delete(slam_thread_pool_t* pool);

extern void slam_thread_pool_init(slam_thread_pool_t* pool, size_t size); /* size = 0: auto alloc */

extern void slam_thread_pool_add_runnable(slam_thread_pool_t* thread_pool, slam_runnable_t* runnable);
extern void slam_thread_pool_remove_runnable(slam_thread_pool_t* thread_pool, slam_runnable_t* runnable);

#endif

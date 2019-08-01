/*
 * \file: slam_thread_pool.c
 * \brief: Created by hushouguo at 17:14:45 Jul 29 2019
 */

#include "slam.h"

struct slam_thread_pool_s {
	size_t size;
	slam_thread_t** threads;
};

slam_thread_pool_t* slam_thread_pool_new() {
	slam_thread_pool_t* pool = (slam_thread_pool_t *) malloc(sizeof(slam_thread_pool_t));
	pool->size = 0;
	pool->threads = nullptr;
	return pool;
}

void slam_thread_pool_delete(slam_thread_pool_t* pool) {
	for (n = 0; n < pool->size; ++n) {
		if (pool->threads[n]) {
			slam_thread_delete(pool->threads[n]);
		}
	}
	slam_free(pool);
}

void slam_thread_pool_init(slam_thread_pool_t* pool, size_t size) {
	size_t n;
	pool->size = size;
	if (pool->size == 0) {
		pool->size = slam_cpus();
	}
	slam_free(pool->threads);
	pool->threads = (slam_thread_t **) malloc(sizeof(slam_thread_t*) * pool->threads);
	for (n = 0; n < pool->size; ++n) {
		pool->threads[n] = slam_thread_new(n);
	}
}

void slam_thread_pool_add_runnable(slam_thread_pool_t* pool, slam_runnable_t* runnable);
void slam_thread_pool_remove_runnable(slam_thread_pool_t* pool, slam_runnable_t* runnable);

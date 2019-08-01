/*
 * \file: slam_thread.c
 * \brief: Created by hushouguo at 17:11:45 Jul 29 2019
 */

#include "slam.h"

struct slam_thread_t {
	size_t id;
	size_t stack_size;
	pthread_t handle;
};

void thread_routine(slam_thread_t* thread) {

}

slam_thread_t* slam_thread_new(size_t id) {
	slam_thread_t* thread = (slam_thread_t *) malloc(sizeof(slam_thread_t));
	thread->id = id;
	thread->stack_size = 0;
	if (pthread_create(&thread->handle, nullptr, &thread_routine, thread)) {
		slam_free(thread);
	}
	return thread;
}

void slam_thread_delete(slam_thread_t* thread) {
	pthread_join(thread->handle, nullptr);
	slam_free(thread);
}


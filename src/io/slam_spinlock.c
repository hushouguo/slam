/*
 * \file: slam_spinlock.c
 * \brief: Created by hushouguo at 16:53:54 Jul 29 2019
 */

#include "slam.h"

struct slam_spinlock_s {
	int lock;
};

slam_spinlock_t* slam_spinlock_new() {
	slam_spinlock_t* spinlock = (slam_spinlock_t *) malloc(sizeof(slam_spinlock_t));
	spinlock->lock = 0;
	return spinlock;
}

void slam_spinlock_delete(slam_spinlock_t* spinlock) {
	slam_free(spinlock);
}

void slam_spinlock_lock(slam_spinlock_t* spinlock) {
	while (__sync_lock_test_and_set(&spinlock->lock, 1));
}

bool slam_spinlock_trylock(slam_spinlock_t* spinlock) {
	return __sync_lock_test_and_set(&spinlock->lock, 1) == 0;
}

void slam_spinlock_unlock(slam_spinlock_t* spinlock) {
	__sync_lock_release(&spinlock->lock);
}


/*
 * \file: slam_spinlock.h
 * \brief: Created by hushouguo at 16:50:43 Jul 29 2019
 */
 
#ifndef __SLAM_SPINLOCK_H__
#define __SLAM_SPINLOCK_H__

typedef struct slam_spinlock_s slam_spinlock_t;

extern slam_spinlock_t* slam_spinlock_new();
extern void slam_spinlock_delete(slam_spinlock_t* spinlock);

extern void slam_spinlock_lock(slam_spinlock_t* spinlock);
extern bool slam_spinlock_trylock(slam_spinlock_t* spinlock);
extern void slam_spinlock_unlock(slam_spinlock_t* spinlock);

#define SPIN_LOCK(spinlock) slam_spinlock_lock(spinlock)
#define SPIN_TRYLOCK(spinlock) slam_spinlock_trylock(spinlock)
#define SPIN_UNLOCK(spinlock) slam_spinlock_unlock(spinlock)

#endif

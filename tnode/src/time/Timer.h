/*
 * \file: Timer.h
 * \brief: Created by hushouguo at Jul 06 2017 20:46:48
 */
 
#ifndef __TIMER_H__
#define __TIMER_H__

BEGIN_NAMESPACE_TNODE {
	struct Timer {
		u32 id;
		u32 milliseconds;	// interval milliseconds
		s32 times;			// -1 means forever
		int ref;			// reference to lua closure
		luaT_Value ctx;
		u64 next_time_point;
	};
}

#endif

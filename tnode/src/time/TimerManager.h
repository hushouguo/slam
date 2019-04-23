/*
 * \file: TimerManager.h
 * \brief: Created by hushouguo at 14:05:42 Apr 23 2019
 */
 
#ifndef __TIMERMANAGER_H__
#define __TIMERMANAGER_H__

BEGIN_NAMESPACE_TNODE {
	class TimerManager {
		public:
			u32 createTimer(u32 milliseconds, s32 times, int ref, const luaT_Value& ctx);
			void removeTimer(u32 timerid);

		public:
			bool setTimerInterval(u32 timerid, u32 milliseconds);
			bool setTimerTimes(u32 timerid, s32 times);
			void checkExpire(std::function<void(Timer*)> func);
			inline u64 firstExpireTime() { return this->_first_expire_time;	}
			
		private:
			u32 _init_timerid = 1;
			volatile u64 _first_expire_time = u64(-1);
			std::list<Timer*> _timerQueue;
			struct timer_node {
				bool operator()(Timer* ts1, Timer* ts2) {
					return ts1->next_time_point < ts2->next_time_point;
				}	
			};
			void dump();
			void pushTimer(Timer* timer);
			void resetFirstExpireTime();
	};
}

#endif

/*
 * \file: TimerManager.cpp
 * \brief: Created by hushouguo at 14:05:53 Apr 23 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "lua/luaT.h"
#include "time/Time.h"
#include "time/Timer.h"
#include "time/TimerManager.h"

BEGIN_NAMESPACE_TNODE {
	u32 TimerManager::createTimer(u32 milliseconds, s32 times, int ref, const luaT_Value& ctx) {
		CHECK_RETURN(milliseconds > 0, 0, "timer interval MUST greater than 0");
		Timer* timer = new Timer();
		sTime.now();
		timer->id = this->_init_timerid++;
		timer->milliseconds = milliseconds;
		timer->times = times;
		timer->ref = ref;
		timer->ctx = ctx;
		timer->next_time_point = sTime.milliseconds() + milliseconds;
		this->pushTimer(timer);		
		return timer->id;
	}

	void TimerManager::pushTimer(Timer* timer) {
		this->_timerQueue.push_back(timer);
		this->_timerQueue.sort(timer_node());
		this->resetFirstExpireTime();
	}

	void TimerManager::resetFirstExpireTime() {
		this->_first_expire_time = u64(-1);
		if (!this->_timerQueue.empty()) {
			Timer* timer = this->_timerQueue.front();
			this->_first_expire_time = timer->next_time_point;
		}
	}
	
	void TimerManager::removeTimer(u32 timerid) {
#if false		
		auto iterator = std::find_if(this->_timerQueue.begin(), this->_timerQueue.end(),
			[timerid](Timer* timer) -> bool {
				return timer->id == timerid;
			});
		if (iterator != this->_timerQueue.end()) {
			delete *iterator;
			this->_timerQueue.erase(iterator);
			this->resetFirstExpireTime();
		}
#else
		this->setTimerTimes(timerid, 0);
#endif		
	}

	void TimerManager::checkExpire(std::function<void(Timer*)> func) {
		sTime.now();		
		std::vector<Timer*> v;
		for (auto timer : this->_timerQueue) {
			if (timer->times == 0) {
				v.push_back(timer);
			}
			else {
				if (timer->next_time_point <= sTime.milliseconds()) {
					func(timer);
					if (timer->times > 0) {
						--timer->times;
						if (timer->times == 0) {
							v.push_back(timer);
						}
					}
					timer->next_time_point = sTime.milliseconds() + timer->milliseconds;
				}
			}
		}

		for (auto timer : v) {
			u32 timerid = timer->id;
			auto iterator = std::find_if(this->_timerQueue.begin(), this->_timerQueue.end(),
				[timerid](Timer* timer) -> bool {
					return timer->id == timerid;
				});
			if (iterator != this->_timerQueue.end()) {
				delete *iterator;
				this->_timerQueue.erase(iterator);
			}
		}

		this->_timerQueue.sort(timer_node());
	}
	
	bool TimerManager::setTimerInterval(u32 timerid, u32 milliseconds) {
		bool rc = false;
		auto iterator = std::find_if(this->_timerQueue.begin(), this->_timerQueue.end(),
			[timerid, milliseconds](Timer* timer) -> bool {
				if (timer->id == timerid) {
					sTime.now();
					timer->milliseconds = milliseconds;
					timer->next_time_point = sTime.milliseconds() + milliseconds;
					return true;
				}
				return false;
			});
		if (iterator != this->_timerQueue.end()) {
			rc = true;
			this->_timerQueue.sort(timer_node());
			this->resetFirstExpireTime();
		}
		return rc;
	}
	
	bool TimerManager::setTimerTimes(u32 timerid, s32 times) {
		bool rc = false;
		auto iterator = std::find_if(this->_timerQueue.begin(), this->_timerQueue.end(),
			[timerid, times](Timer* timer) -> bool {
				if (timer->id == timerid) {
					sTime.now();
					timer->times = times;
					return true;
				}
				return false;
			});
		if (iterator != this->_timerQueue.end()) {
			rc = true;
		}
		return rc;
	}

	void TimerManager::dump() {
		for (auto& timer : this->_timerQueue) {
			Debug << "	  timer: " << timer->id << ", " << timer->times << ", milliseconds: " << timer->milliseconds;
		}
	}
}


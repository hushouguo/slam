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
		this->_timerQueue.erase(std::remove_if(this->_timerQueue.begin(), this->_timerQueue.end(), 
			[timerid](Timer* timer) -> bool {
				return timer->id == timerid;
				}), 
				this->_timerQueue.end());
		this->resetFirstExpireTime();
	}

	void TimerManager::tickTimer(Timer* timer) {
		if (timer->times > 0) {
			--timer->times;
			if (timer->times == 0) {
				//Debug << "timer: " << timer->id << " exhause times";
				SafeDelete(timer);
				return;
			}
		}

		sTime.now();
		timer->next_time_point = sTime.milliseconds() + timer->milliseconds;
		
		this->pushTimer(timer);
	}

	Timer* TimerManager::getTimerExpire() {
		Timer* timer = nullptr;
		sTime.now();
		if (!this->_timerQueue.empty()) {
			timer = this->_timerQueue.front();
			if (timer->next_time_point <= sTime.milliseconds()) {
				this->_timerQueue.pop_front();
			}
			else {
				timer = nullptr;
			}
		}
		if (timer) {
			this->resetFirstExpireTime();
		}
		return timer;
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
			Debug << "	  timer: " << timer->id << ", " << timer->next_time_point << ", milliseconds: " << timer->milliseconds;
		}
	}
}


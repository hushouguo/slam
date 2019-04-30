/*
 * \file: Timer.h
 * \brief: Created by hushouguo at Jul 06 2017 20:46:48
 */
 
#ifndef __TIMER_H__
#define __TIMER_H__

BEGIN_NAMESPACE_SLAM {
	// base on milliseconds
	class Timer {
		public:
			Timer()	: _last_scale(sTime.milliseconds()), _interval_milliseconds(0) {
			}
			
			Timer(uint32_t milliseconds) : _last_scale(sTime.milliseconds()), _interval_milliseconds(milliseconds) {
				//assert(this->Interval() > 0);
			}

		public:
			inline void resetInterval(uint32_t milliseconds) { 
				this->_interval_milliseconds = milliseconds; 
			}
			
			inline uint32_t Interval() { 
				return this->_interval_milliseconds; 
			}
			
			inline bool operator()() {
				if ((sTime.milliseconds() - this->_last_scale) >= this->_interval_milliseconds) {
					this->_last_scale = sTime.milliseconds();
					return true;
				}
				return false;
			}

			inline void resetScale() {
				this->_last_scale = sTime.milliseconds();
			}

		private:
			uint64_t _last_scale = 0;
			uint32_t _interval_milliseconds = 0;
	};	
}

#endif

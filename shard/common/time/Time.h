/*
 * \file: Time.h
 * \brief: Created by hushouguo at Jul 06 2017 20:46:33
 */
 
#ifndef __TIME_H__
#define __TIME_H__

BEGIN_NAMESPACE_SLAM {
	class Time {
		public:
			Time();
			Time(u64 milliseconds);
			Time(u64 second_part, u64 millisecond_part);
			Time(const Time& t);

		public:
			void now();
			inline u64 seconds() const { return this->_tv.tv_sec; }
			inline u64 milliseconds() const { return this->seconds() * 1000 + this->millisecondsPart(); }
			inline u64 millisecondsPart() const { return this->_tv.tv_usec / 1000; }
			inline u64 microsecondsPart() const { return this->_tv.tv_usec; }
			
		public:
			void operator =(u64 milliseconds);
			void operator =(const Time& t);
			u64  operator +(u64 milliseconds);
			u64  operator +(const Time& t);
			u64  operator -(u64 milliseconds);
			u64  operator -(const Time& t);
			bool operator >(const Time& t);
			bool operator <(const Time& t);			
			
		private:
			struct timeval _tv = {tv_sec:0, tv_usec:0};
	};
}

#define sTime slam::Singleton<slam::Time>::getInstance()

#endif

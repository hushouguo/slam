/*
 * \file: Service.h
 * \brief: Created by hushouguo at 14:51:12 Mar 25 2019
 */
 
#ifndef __SERVICE_H__
#define __SERVICE_H__

BEGIN_NAMESPACE_TNODE {
	class Service : public Runnable {
		public:
			Service(u32 id) : Runnable(id) {
			}
			~Service();
			const char* getClassName() override { return "Service"; }

		public:
			inline bool isstop() { return this->_isstop; }
			inline lua_State* luaState() { return this->_L; }
			inline const std::string& entryfile() { return this->_entryfile; }

		public:
			bool init(const char* entryfile);
			void stop();
			void run() override;
			bool need_schedule();

		private:
			bool _isstop = false;
			bool _isinit = false;
			std::string _entryfile;
			lua_State* _L = nullptr;
			void cleanup();
		
		// network
		private:
			LockfreeQueue<const void*> _msgQueue;
			MessageParser* _msgParser = nullptr;
			
		public:
			inline void pushMessage(const void* netmsg) {
				this->_msgQueue.push_back(netmsg);
			}
			inline MessageParser* msgParser() { return this->_msgParser; }
			
		
		// timer handle
		public:
			u32 regtimer(u32 milliseconds, s32 times, int ref, const luaT_Value& ctx);
		
		private:
			u32 _init_timerid = 1;
			struct timer_struct {
				u32 id;
				u32 milliseconds;
				s32 times;
				int ref;
				luaT_Value ctx;
				u64 next_time_point;
			};
			Spinlocker _locker;
			std::list<timer_struct*> _timerQueue;
			struct timer_node {
				bool operator()(timer_struct* ts1, timer_struct* ts2) {
					return ts1->next_time_point < ts2->next_time_point;
				}	
			};
			inline void pushTimer(timer_struct* ts) {
				this->_locker.lock();
				this->_timerQueue.push_back(ts);
				this->_timerQueue.sort(timer_node());
				this->_locker.unlock();
				//Debug << "Service: " << this->id << " all of timers";
				//for (auto& ts : this->_timerQueue) {
				//	Debug << "    ts: " << ts->next_time_point << ", milliseconds: " << ts->milliseconds;
				//}
			}
			inline timer_struct* popTimer() {
				timer_struct* ts = nullptr;
				this->_locker.lock();
				if (!this->_timerQueue.empty()) {
					ts = this->_timerQueue.front();
					this->_timerQueue.pop_front();
				}
				this->_locker.unlock();
				return ts;
			}
			inline bool timerExpire() {
				sTime.now();
				bool rc = false;
				this->_locker.lock();
				if (!this->_timerQueue.empty()) {
					timer_struct* ts = this->_timerQueue.front();
					rc = ts->next_time_point <= sTime.milliseconds();
				}
				this->_locker.unlock();
				return rc;
			}
	};
}

#endif

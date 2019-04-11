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
			const char* getClassName() override { return "Service"; }

		public:
			inline bool isstop() { return this->_isstop; }
			inline lua_State* luaState() { return this->_L; }
			inline MessageParser* messageParser() { return this->_messageParser; }
			inline const std::string& entryfile() { return this->_entryfile; }
			inline void pushMessage(const void* netmsg) {
				this->_msgQueue.push_back(netmsg);
			}

		public:
			bool init(const char* entryfile);
			void stop();
			void run() override;
			bool need_schedule();

		private:
			bool _isstop = false;
			std::string _entryfile;
			lua_State* _L = nullptr;
			MessageParser* _messageParser = nullptr;

		private:
			LockfreeQueue<const void*> _msgQueue;

		// timer handle
		public:
			void regtimer(u32 milliseconds, int ref, const luaT_Value& ctx);
		
		private:
			enum timer_type {
				timer_once		=	1, 
				timer_forever	=	2,
			};
			struct timer_struct {
				u64 next_time_point;
				u32 milliseconds;
				int ref;
				luaT_Value ctx;
				timer_type type;
			};
			Spinlocker _locker;
			std::list<timer_struct*> _timerQueue;
			struct timer_node {
				bool operator()(timer_struct* ts1, timer_struct* ts2) {
					return ts1->next_time_point > ts2->next_time_point;
				}	
			};
			inline void pushTimer(timer_struct* ts) {
				this->_locker.lock();
				this->_timerQueue.push_back(ts);
				this->_timerQueue.sort(timer_node());
				this->_locker.unlock();
				for (auto& ts : this->_timerQueue) {
					Debug << "ts: " << ts->next_time_point << ", milliseconds: " << ts->milliseconds;
				}
			}
			inline timer_struct* getTimer() {
				assert(!this->_timerQueue.empty());
				this->_locker.lock();
				timer_struct* ts = this->_timerQueue.front();
				this->_timerQueue.pop_front();
				this->_locker.unlock();
				return ts;
			}
	};
}

#endif

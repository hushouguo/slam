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
			inline bool isinit() { return this->_isinit; }
			inline lua_State* luaState() { return this->_L; }
			inline const std::string& entryfile() { return this->_entryfile; }
			inline TimerManager& timerManager() { return this->_timerManager; }

		public:
			bool init(const char* entryfile);
			void stop();
			void run() override;
			bool need_schedule();

		public:
			bool entry_init();
			void entry_destroy();

		private:
			bool _isstop = false, _isinit = false;
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

		// timer
		private:
			TimerManager _timerManager;
	};
}

#endif

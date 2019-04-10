/*
 * \file: Service.h
 * \brief: Created by hushouguo at 14:51:12 Mar 25 2019
 */
 
#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "tools/Runnable.h"
#include "tools/LockfreeQueue.h"
#include "lua/MessageParser.h"

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

		// message handle
		public:
			u32 dispatch(u64 entityid, u32 msgid);
			bool msgParser(const ServiceMessage* msg);
			void pushMessage(const void* netmsg);
			
		private:
			LockfreeQueue<const void*> _msgQueue;

		// timer handle
		public:
			void regtimer(u32 milliseconds, int ref);
		
		private:
			enum timer_type {
				timer_once		=	1, 
				timer_forever	=	2,
			};
			struct timer_struct {
				u64 last_check;
				u32 milliseconds;
				int ref;
				timer_type type;
			};
			LockfreeQueue<timer_struct*> _timerQueue;
	};
}

#endif

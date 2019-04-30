/*
 * \file: Service.cpp
 * \brief: Created by hushouguo at 12:46:57 Mar 26 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "tools/Entry.h"
#include "tools/ThreadPool.h"
#include "tools/Runnable.h"
#include "tools/Spinlocker.h"
#include "tools/LockfreeQueue.h"
#include "tools/Registry.h"
#include "message/ServiceMessage.h"
#include "message/MessageParser.h"
#include "config/Config.h"
#include "lua/luaT.h"
#include "time/Time.h"
#include "time/Timer.h"
#include "time/TimerManager.h"
#include "lua/luaT_message_parser.h"
#include "lua/luaT_entry.h"
#include "db/Easydb.h"
#include "service/Service.h"
#include "net/NetworkManager.h"


BEGIN_NAMESPACE_TNODE {
	bool Service::init(const char* entryfile) {
		//
		// network init
		this->_msgParser = new MessageParser();

		//
		// lua state init & execute entryfile
		this->_L = luaT_newstate();
		luaT_setOwner(this->_L, this->id);
		this->_entryfile = entryfile;
		//if (!luaT_execFile(this->_L, this->_entryfile.c_str())) { return false; }
		
		return true;
	}

	bool Service::entry_init() {
		CHECK_RETURN(!this->_isinit, false, "reentry_init");
		if (!luaT_execFile(this->_L, this->_entryfile.c_str())) {
			return false; 
		}
		luaT_entry_init(this->_L, this->id);
		return this->_isinit = true;
	}
	
	void Service::entry_destroy() {
		assert(this->_L);
		luaT_entry_destroy(this->_L);
	}

	Service::~Service() {
		this->cleanup();
	}
	
	void Service::cleanup() {
		this->stop();

		//
		// call destroy
		this->entry_destroy();
		
		//
		// network cleanup
		while (!this->_msgQueue.empty()) {
			const void* netmsg = this->_msgQueue.pop_front();
			sNetworkManager.easynet()->releaseMessage(netmsg);
		}
		this->_msgQueue.clear();

		SafeDelete(this->_msgParser);
		
		//
		// close lua state
		luaT_close(this->_L);
	}

	void Service::stop() {
		this->_isstop = true;
	}
	
	void Service::run() {
		if (this->isstop()) {
			return;
		}

		if (!this->_isinit) {
			if (!this->entry_init()) {
				this->stop();
				return; 
			}
		}
		
		while (!this->_msgQueue.empty()) {
			const void* netmsg = this->_msgQueue.pop_front();
			assert(netmsg);
			luaT_entry_msgParser(this->luaState(), netmsg, this->msgParser());
			sNetworkManager.easynet()->releaseMessage(netmsg);
		}

		this->timerManager().checkExpire([this](Timer* timer) {
			luaT_entry_timer_expire(this->luaState(), timer->id, timer->ref, timer->ctx);			
		});
	}

	bool Service::need_schedule() {
		sTime.now();
		return !this->isstop() 
			&& 
				// still not execute entryfile
			(!this->_isinit
				// network message arrived
				|| !this->_msgQueue.empty()
				// timer expire
				|| this->timerManager().firstExpireTime() <= sTime.milliseconds()
				);
	}
}


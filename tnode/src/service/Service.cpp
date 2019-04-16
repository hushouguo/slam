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
#include "tools/LockfreeMap.h"
#include "tools/Registry.h"
#include "message/ServiceMessage.h"
#include "config/Config.h"
#include "lua/luaT.h"
#include "lua/luaT_message_parser.h"
#include "lua/luaT_entry.h"
#include "time/Time.h"
#include "service/Service.h"
#include "net/NetworkManager.h"


BEGIN_NAMESPACE_TNODE {
	bool Service::init(const char* entryfile) {
		//
		// network init
		this->_msgParser = luaT_message_parser_creator::create();

		//
		// lua state init & execute entryfile
		this->_L = luaT_newstate();
		luaT_setOwner(this->_L, this->id);
		this->_entryfile = entryfile;
		if (!luaT_execFile(this->_L, this->_entryfile.c_str())) { return false; }
		
		return true;
	}

	Service::~Service() {
		this->cleanup();
	}
	
	void Service::cleanup() {
		this->stop();

		//
		// db cleanup
		for (auto& easydb : this->_dbs) {
			SafeDelete(easydb);
		}
		this->_dbs.clear();
		
		//
		// network cleanup
		while (!this->_msgQueue.empty()) {
			const void* netmsg = this->_msgQueue.pop_front();
			sNetworkManager.easynet()->releaseMessage(netmsg);
		}
		this->_msgQueue.clear();		
		SafeDelete(this->_msgParser);
		
		//
		// timer cleanup
		for (auto& ts : this->_timerQueue) {
			SafeDelete(ts);
		}
		this->_timerQueue.clear();

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
		
		while (!this->_msgQueue.empty()) {
			const void* netmsg = this->_msgQueue.pop_front();
			assert(netmsg);
			luaT_entry_msgParser(this->luaState(), netmsg, this->msgParser());
			sNetworkManager.easynet()->releaseMessage(netmsg);
		}

		sTime.now();
		while (!this->_timerQueue.empty()) {
			timer_struct* ts = this->popTimer();
			if (ts->next_time_point > sTime.milliseconds()) {
				this->pushTimer(ts);
				break;
			}
			luaT_entry_timer_expire(this->luaState(), ts->id, ts->ref, ts->ctx);
			--ts->times;
			if (ts->times == 0) {
				//Debug << "timer: " << ts->id << " exhause times";
				SafeDelete(ts);
				//TODO: cancel lua_State reference
			}
			else {
				ts->next_time_point = sTime.milliseconds() + ts->milliseconds;
				this->pushTimer(ts);
				//Debug << "timer: " << ts->id << " leave times: " << ts->times;
			}
		}
	}

	bool Service::need_schedule() {
		return !this->isstop() && (!this->_msgQueue.empty() || this->hasTimerExpire());
	}
    
	u32 Service::regtimer(u32 milliseconds, s32 times, int ref, const luaT_Value& ctx) {
		CHECK_RETURN(milliseconds > 0, 0, "timer interval MUST greater than 0");
		timer_struct* ts = new timer_struct();
		sTime.now();
		ts->id = this->_init_timerid++;
		ts->milliseconds = milliseconds;
		ts->times = times;
		ts->ref = ref;
		ts->ctx = ctx;
		ts->next_time_point = sTime.milliseconds() + milliseconds;
		Debug << "regtimer: " << ts->id << " interval: " << ts->milliseconds;
		this->pushTimer(ts);
		return ts->id;
	}
}


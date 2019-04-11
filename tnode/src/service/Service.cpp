/*
 * \file: Service.cpp
 * \brief: Created by hushouguo at 12:46:57 Mar 26 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "message/ServiceMessage.h"
#include "config/Config.h"
#include "lua/luaT.h"
#include "lua/MessageParser.h"
#include "lua/luaT_entry.h"
#include "time/Time.h"
#include "service/Service.h"
#include "net/NetworkManager.h"


BEGIN_NAMESPACE_TNODE {
	bool Service::init(const char* entryfile) {
		this->_messageParser = MessageParserCreator::create();
		this->_L = luaT_newstate();
		luaT_setOwner(this->_L, this->id);
		this->_entryfile = entryfile;
		/* run entry script file */
		if (!luaT_execFile(this->_L, this->_entryfile.c_str())) { return false; }		
		return true;
	}

	void Service::stop() {
		if (!this->_isstop) {
			this->_isstop = true;
			SafeDelete(this->_messageParser);
			luaT_close(this->_L);
			while (!this->_msgQueue.empty()) {
				const void* netmsg = this->_msgQueue.pop_front();
				sNetworkManager.easynet()->releaseMessage(netmsg);
			}
		}
	}

	void Service::run() {
		while (!this->isstop() && !this->_msgQueue.empty()) {
			const void* netmsg = this->_msgQueue.pop_front();
			assert(netmsg);
			luaT_entry_msgParser(this->luaState(), netmsg, this->messageParser());
			sNetworkManager.easynet()->releaseMessage(netmsg);
		}

		sTime.now();
		while (!this->isstop() && !this->_timerQueue.empty()) {
			timer_struct* ts = this->getTimer();
			if (ts->next_time_point > sTime.milliseconds()) {
				this->pushTimer(ts);
				break;
			}
			luaT_entry_timer_expire(this->luaState(), ts->ref);
			if (ts->type == timer_forever) {
				ts->next_time_point = sTime.milliseconds() + ts->milliseconds;
				this->pushTimer(ts);
			}
			else if (ts->type == timer_once) {
				SafeDelete(ts);
			}
		}
	}

	bool Service::need_schedule() {
		return !this->isstop() && !this->_msgQueue.empty() && !this->_timerQueue.empty();
	}
    
	void Service::regtimer(u32 milliseconds, int ref) {
		CHECK_RETURN(milliseconds > 0, void(0), "timer interval MUST greater than 0");
		timer_struct* ts = new timer_struct();
		sTime.now();
		ts->next_time_point = sTime.milliseconds() + milliseconds;
		ts->milliseconds = milliseconds;
		ts->type = timer_forever;
		ts->ref = ref;
		this->pushTimer(ts);
	}
}


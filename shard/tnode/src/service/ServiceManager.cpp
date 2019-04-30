/*
 * \file: ServiceManager.cpp
 * \brief: Created by hushouguo at 16:36:03 Mar 26 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "tools/Entry.h"
#include "tools/Spinlocker.h"
#include "tools/LockfreeQueue.h"
#include "tools/ThreadPool.h"
#include "tools/Runnable.h"
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
#include "service/ServiceManager.h"
#include "net/NetworkManager.h"

#define VALID_SERVICE(sid)	((sid) >= 0 && (sid) < TNODE_SERVICE_MAX_NUMBER)

BEGIN_NAMESPACE_TNODE {
	ServiceManager::ServiceManager() {
		memset(this->_services, 0, sizeof(this->_services));
	}
	
	bool ServiceManager::init(const char* entryfile) {
		Service* service = this->newservice(entryfile);
		CHECK_RETURN(service, false, "ServiceManager init failure");
		this->_entrysid = service->id;
		return true;
	}
	
	Service* ServiceManager::newservice(const char* entryfile, bool schedule) {
		u32 sid = this->_autosid++;
		CHECK_RETURN(VALID_SERVICE(sid), nullptr, "number of service overflow");
		assert(this->_services[sid] == nullptr);
		Service* service = new Service(sid);
		bool result = service->init(entryfile);
		if (!result) {
			SafeDelete(service);
			return nullptr;
		}
		this->_services[sid] = service;
		if (schedule) {
			service->schedule();
		}
		return service;
	}

	void ServiceManager::stop() {
		for (auto service : this->_services) {
			SafeDelete(service);
		}
		memset(this->_services, 0, sizeof(this->_services));
	}

	Service* ServiceManager::getService(u32 sid) {
		assert(VALID_SERVICE(sid));
		return this->_services[sid];
	}
	
	Service* ServiceManager::getService(lua_State* L) {
		u32 sid = luaT_getOwner(L);
		Service* service = this->getService(sid);
		assert(service);
		return service;
	}

	bool ServiceManager::pushMessage(const void* netmsg) {
		assert(VALID_SERVICE(this->_entrysid));
		Service* entryservice = this->_services[this->_entrysid];
		CHECK_RETURN(entryservice, false, "Not found entry service: %d", this->_entrysid);

		//
		// perhaps entryservice still not complete to init
		while (!entryservice->isinit()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		
		u32 sid = luaT_entry_dispatch(entryservice->luaState(), netmsg);
		CHECK_RETURN(VALID_SERVICE(sid), false, "entry service: %s call `dispatch` error: %d", entryservice->entryfile().c_str(), sid);

		Service* service = this->_services[sid];
		CHECK_RETURN(service, false, "Not found dispatch service: %d", sid);
		CHECK_RETURN(!service->isstop(), false, "dispatch service: %d isstop", sid);
		
		service->pushMessage(netmsg);
		//service->schedule();

		return true;
	}
	
	bool ServiceManager::exitservice(u32 sid) {
		Debug << "exit service: " << sid;
		CHECK_RETURN(VALID_SERVICE(sid), false, "exitservice: %d invalid", sid);
		Service* service = this->_services[sid];
		CHECK_RETURN(service, false, "exitservice, not found service: %d", sid);
		service->stop();
		return true;
	}

	void ServiceManager::schedule() {
		for (int sid = 0; sid < TNODE_SERVICE_MAX_NUMBER; ++sid) {
			Service* service = this->_services[sid];
			if (!service) {	continue; }
			if (service->isstop()) {
				if (!service->isrunning()) {
					this->_services[sid] = nullptr;
					Debug << "destroy service: " << service->id;
					SafeDelete(service);
				}
			}
			else if (service->need_schedule()) {
				service->schedule();
			}
		}
	}

	u64 ServiceManager::getFirstTimerExpire() {
		u64 expireValue = TIMER_INFINITE;
		for (int sid = 0; sid < TNODE_SERVICE_MAX_NUMBER; ++sid) {
			Service* service = this->_services[sid];
			if (!service) {	continue; }
			u64 value = service->timerManager().firstExpireTime();
			if (value < expireValue) {
				expireValue = value;
			}
		}
		return expireValue;
	}
	
	INITIALIZE_INSTANCE(ServiceManager);
}

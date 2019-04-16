/*
 * \file: ServiceManager.cpp
 * \brief: Created by hushouguo at 16:36:03 Mar 26 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "tools/Entry.h"
#include "tools/Spinlocker.h"
#include "tools/LockfreeQueue.h"
#include "tools/LockfreeMap.h"
#include "tools/ThreadPool.h"
#include "tools/Runnable.h"
#include "message/ServiceMessage.h"
#include "time/Time.h"
#include "lua/luaT.h"
#include "lua/luaT_message_parser.h"
#include "lua/luaT_entry.h"
#include "db/Easydb.h"
#include "service/Service.h"
#include "service/ServiceManager.h"
#include "net/NetworkManager.h"

BEGIN_NAMESPACE_TNODE {
	bool ServiceManager::init(const char* entryfile) {
		Service* service = this->newservice(entryfile);
		CHECK_RETURN(service, false, "ServiceManager init failure");
		this->_initid = service->id;
		return true;
	}
	
	void ServiceManager::stop() {
		for (auto& i : this->_services) {
			Service* service = i.second;
			SafeDelete(service);
		}
		this->_services.clear();
	}

	bool ServiceManager::pushMessage(const void* netmsg) {	
		Service* initservice = this->getService(this->_initid);
		CHECK_RETURN(initservice, false, "Not found initservice: %d", this->_initid);

		u32 sid = luaT_entry_dispatch(initservice->luaState(), netmsg);
		CHECK_RETURN(sid != ILLEGAL_SERVICE, false, "initservice: %s call `dispatch` error", initservice->entryfile().c_str());

		Service* service = this->getService(sid);
		CHECK_RETURN(service, false, "Not found dispatch service: %d", sid);
		CHECK_RETURN(!service->isstop(), false, "service: %d isstop", sid);
		
		service->pushMessage(netmsg);			
		this->schedule(service);	// schedule service right now

		return true;
	}
	
	Service* ServiceManager::newservice(const char* entryfile) {
		u32 sid = this->_autoid++;
		assert(this->getService(sid) == nullptr);
		Service* service = new Service(sid);
		this->insertService(sid, service);
		bool result = service->init(entryfile);
		if (!result) {
			this->removeService(sid);
			SafeDelete(service);
		}
		Debug << "new service: " << sid;
		return service;
	}

	bool ServiceManager::exitservice(u32 sid) {
		Debug << "exit service: " << sid;
		Service* service = this->getService(sid);
		CHECK_RETURN(service, false, "not found service: %d", sid);
		service->stop();
		return true;
	}

	void ServiceManager::schedule(Service* service) {
		if (service->need_schedule()) {
			service->schedule();
		}
	}

	void ServiceManager::schedule() {
		std::vector<Service*> deprecated;
		
		this->_locker.lock();
		for (auto& i : this->_services) {
			Service* service = i.second;
			if (service->isstop()) {
				if (!service->isrunning()) {
					deprecated.push_back(service);
				}
			}
			else {
				this->schedule(service);
			}
		}
		this->_locker.unlock();

		if (!deprecated.empty()) {
			for (auto& service : deprecated) {
				assert(service->isstop());
				assert(!service->isrunning());
				this->removeService(service->id);
				Debug << "destroy service: " << service->id;
				SafeDelete(service);
			}
		}
	}
	
	INITIALIZE_INSTANCE(ServiceManager);
}

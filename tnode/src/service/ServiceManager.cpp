/*
 * \file: ServiceManager.cpp
 * \brief: Created by hushouguo at 16:36:03 Mar 26 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "message/ServiceMessage.h"
#include "lua/luaT.h"
#include "lua/luaT_entry.h"
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
		while (!this->_services.empty()) {
			Service* service = this->_services.pop_front();
			SafeDelete(service);
		}
	}

	bool ServiceManager::pushMessage(const void* netmsg) {	
		Service* initservice = this->getService(this->_initid);
		CHECK_RETURN(initservice, false, "Not found initservice: %d", this->_initid);

		u32 sid = luaT_entry_dispatch(initservice->luaState(), netmsg);
		CHECK_RETURN(sid != ILLEGAL_SERVICE, false, "initservice: %s call `dispatch` error", initservice->entryfile().c_str());

		Service* service = this->getService(sid);
		CHECK_RETURN(service, false, "Not found dispatch service: %d", sid);
		
		service->pushMessage(netmsg);			
		this->schedule(service);	// schedule service right now

		return true;
	}
	
	Service* ServiceManager::newservice(const char* entryfile) {
		u32 sid = this->_autoid++;
		assert(this->getService(sid) == nullptr);
		Service* service = new Service(sid);
		this->_services.insert(sid, service);
		bool result = service->init(entryfile);
		if (!result) {
			this->_services.eraseKey(sid);
			SafeDelete(service);
		}
		Debug << "new service: " << sid;
		return service;
	}

	bool ServiceManager::exitservice(u32 sid) {
		Service* service = this->getService(sid);
		CHECK_RETURN(service, false, "not found service: %d", sid);
		//this->_services.eraseKey(sid);
		//SafeDelete(service);
		service->stop();
		Debug << "exit service: " << sid;
		return true;
	}

	void ServiceManager::schedule(Service* service) {
		if (service->need_schedule()) {
			service->schedule();
		}
	}

	void ServiceManager::schedule() {
		std::vector<Service*> schelist, shutlist;
		this->_services.traverse([&schelist, &shutlist](u32 sid, Service* service) {
			if (service->isstop()) {
				shutlist.push_back(service);
			}
			else {
				schelist.push_back(service);
			}
		});

		for (auto& service : shutlist) {
			this->_services.eraseKey(service->id);
			Debug << "destroy service: " << service->id;
			SafeDelete(service);
		}
		
		for (auto& service : schelist) {
			this->schedule(service);
		}
	}
	
	INITIALIZE_INSTANCE(ServiceManager);
}

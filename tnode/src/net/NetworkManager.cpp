/*
 * \file: NetworkManager.cpp
 * \brief: Created by hushouguo at 17:02:40 Apr 02 2019
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
#include "time/Time.h"
#include "config/Config.h"
#include "lua/luaT.h"
#include "lua/MessageParser.h"
#include "message/ServiceMessage.h"
#include "service/Service.h"
#include "service/ServiceManager.h"
#include "net/NetworkManager.h"

BEGIN_NAMESPACE_TNODE {
	void NetworkManager::init() {
		this->_easynet = Easynet::createInstance([](const void* buffer, size_t len) -> int {
				ServiceMessage* msg = (ServiceMessage*) buffer;
				return len < sizeof(ServiceMessage) || len < msg->len ? 0 : msg->len;
				});
	}
	
	void NetworkManager::stop() {
		this->_easynet->stop();
		//SafeDelete(this->_easynet);
	}

	void NetworkManager::run() {
		while (!sConfig.halt) {
			const void* netmsg = this->_easynet->getMessage(nullptr);
			if (!netmsg) {
				break;
			}
			if (!sServiceManager.pushMessage(netmsg)) {
				this->_easynet->releaseMessage(netmsg);
			}
		}
	}

	INITIALIZE_INSTANCE(NetworkManager);
}

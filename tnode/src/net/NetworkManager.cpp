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
#include "message/ServiceMessage.h"
#include "message/MessageParser.h"
#include "time/Time.h"
#include "config/Config.h"
#include "lua/luaT.h"
#include "lua/luaT_message_parser.h"
#include "db/Easydb.h"
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

	SOCKET NetworkManager::createServer(const char* name, const char* address, int port) {
		CHECK_RETURN(!this->_registeSockets.containKey(name), -1, "duplicate registe server: %s", name);
		SOCKET fd = this->easynet()->createServer(address, port);
		if (fd != -1) {
			this->_registeSockets.insert(name, fd);
		}
		return fd;
	}
	
	SOCKET NetworkManager::createClient(const char* name, const char* address, int port) {
		CHECK_RETURN(!this->_registeSockets.containKey(name), -1, "duplicate registe client: %s", name);
		SOCKET fd = this->easynet()->createClient(address, port);
		if (fd != -1) {
			this->_registeSockets.insert(name, fd);
		}
		return fd;
	}

	SOCKET NetworkManager::findSocket(const char* name) {
		return this->_registeSockets.containKey(name) ? this->_registeSockets.find(name) : -1;
	}

	INITIALIZE_INSTANCE(NetworkManager);
}

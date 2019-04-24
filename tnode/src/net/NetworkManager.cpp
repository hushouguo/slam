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
#include "config/Config.h"
#include "lua/luaT.h"
#include "time/Time.h"
#include "time/Timer.h"
#include "time/TimerManager.h"
#include "lua/luaT_message_parser.h"
#include "db/Easydb.h"
#include "service/Service.h"
#include "service/ServiceManager.h"
#include "net/NetworkManager.h"

using Clock = std::chrono::high_resolution_clock;
using Ms = std::chrono::milliseconds;
using Sec = std::chrono::seconds;
template<class Duration> using TimePoint = std::chrono::time_point<Clock, Duration>;

BEGIN_NAMESPACE_TNODE {
	void NetworkManager::init() {
		this->_easynet = Easynet::createInstance(
			[](const void* buffer, size_t len) -> int {
				ServiceMessage* msg = (ServiceMessage*) buffer;
				return len < sizeof(ServiceMessage) || len < msg->len ? 0 : msg->len;
				}, 
			[this]() {
				this->wakeup();
				});
	}
	
	void NetworkManager::wakeup() {
		this->_cond.notify_all();	
	}

	void NetworkManager::stop() {
		this->wakeup();
		this->_easynet->stop();
	}

	void NetworkManager::run() {
		std::unique_lock<std::mutex> locker(this->_mtx);
		u64 expireValue = sServiceManager.getFirstTimerExpire();
		if (expireValue == TIMER_INFINITE) {
			this->_cond.wait(locker);
		}
		else {
			sTime.now();
			TimePoint<Ms> timeout(Ms(sTime.milliseconds() + expireValue));
			this->_cond.wait_until(locker, timeout);			
		}

		Debug << "NetworkManager wakeup: " << expireValue;

		this->dispatchMessage();
	}

	void NetworkManager::dispatchMessage() {
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

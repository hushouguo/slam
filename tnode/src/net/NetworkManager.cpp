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
using TimePoint = std::chrono::time_point<Clock>;
//template<class Duration> using TimePoint = std::chrono::time_point<Clock, Duration>;

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
		u64 expireValue = sServiceManager.getFirstTimerExpire();
		if (this->_easynet->getMessageSize() == 0) {
			std::unique_lock<std::mutex> locker(this->_mtx);
			if (expireValue == TIMER_INFINITE) {
				this->_cond.wait(locker);
			}
			else {
				const Clock::duration duration = std::chrono::milliseconds(expireValue);
				const TimePoint timeout(duration);
				this->_cond.wait_until(locker, timeout);			
			}
		}
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

	INITIALIZE_INSTANCE(NetworkManager);
}

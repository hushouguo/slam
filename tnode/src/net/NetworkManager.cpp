/*
 * \file: NetworkManager.cpp
 * \brief: Created by hushouguo at 17:02:40 Apr 02 2019
 */

#include "tnode.h"
#include "config/Config.h"
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
		SafeDelete(this->_easynet);
	}

	void NetworkManager::run() {
		while (!sConfig.halt) {
			SOCKET fd = -1;
			const void* netmsg = this->_easynet->getMessage(&fd);
			if (!netmsg) {
				break;
			}
			sServiceManager.pushMessage(fd, netmsg);
#if 0
			size_t len = 0;
			const void* payload = this->_easynet->getMessageContent(netmsg, &len);
			assert(len >= sizeof(ServiceMessage));
			const ServiceMessage* msg = (const ServiceMessage*) payload;
			assert(len == msg->len);
			sServiceManager.pushMessage(fd, msg);
#endif			
		}
	}

	INITIALIZE_INSTANCE(NetworkManager);
}

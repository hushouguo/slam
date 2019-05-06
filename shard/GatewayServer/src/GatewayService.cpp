/*
 * \file: GatewayService.cpp
 * \brief: Created by hushouguo at 13:28:35 May 06 2019
 */

#include "common.h"
#include "ClientTask.h"
#include "ClientTaskManager.h"
#include "SceneClient.h"
#include "SceneClientManager.h"
#include "GatewayPlayer.h"
#include "GatewayPlayerManager.h"
#include "GatewayService.h"
#include "MainProcess.h"

BEGIN_NAMESPACE_SLAM {
	bool GatewayService::init(const char* address, int port, size_t number) {
		//assert(this->_easynet == nullptr && ILLEGAL_SOCKET(this->_socket));	
		if (number == 0) {
			number = cpus();
		}
		assert(number > 0);
		for (size_t n = 0; n < number; ++n) {
			Easynet* easynet = Easynet::createInstance(
					[](const void* buffer, size_t len) -> int {
						CommonMessage* msg = (CommonMessage*) buffer;
						return len < sizeof(CommonMessage) || len < msg->len ? 0 : msg->len;
					}, 
					[]() {
						sMainProcess.wakeup();
					});
			SOCKET socket = easynet->createServer(address, port);
			CHECK_RETURN(!ILLEGAL_SOCKET(socket), false, "createServer:(%s:%d) failure", address, port);
			this->_easynets.push_back(easynet);
		}
		Debug << "GatewayService listening on " << address << ":" << port << ", Easynet: " << number;
		return true;
	}

	void GatewayService::stop() {
		//
		// cleanup Easynet
		for (auto& easynet : this->_easynets) {
			SafeDelete(easynet);
		}
		this->_easynets.clear();
	}

	void GatewayService::run() {
		for (auto& easynet : this->_easynets) {
			this->run(easynet);
		}
	}
			
	void GatewayService::run(Easynet* easynet) {
		//
		// handle connection state
		while (!sConfig.halt) {
			bool state = false;
			SOCKET socket = easynet->getSocketState(&state);
			if (ILLEGAL_SOCKET(socket)) {
				break;
			}
			if (state) {
				ClientTask* task = new ClientTask(easynet, socket);
				if (!sClientTaskManager.add(task)) {
					SafeDelete(task);
				}
				Debug << "new client task arrived: " << socket;
			}
			else {
				Alarm << "lost client task: " << socket;
				ClientTask* task = sClientTaskManager.find(socket);
				if (task) {
					sGatewayPlayerManager.lostClient(task);					
					sClientTaskManager.remove(task);
					SafeDelete(task);
				}
			}
		}

		//
		// handle message
		while (!sConfig.halt) {
			SOCKET socket = EASYNET_ILLEGAL_SOCKET;
			const void* netmsg = easynet->receiveMessage(&socket);
			if (!netmsg) {
				break;
			}
			assert(!ILLEGAL_SOCKET(socket));
			ClientTask* task = sClientTaskManager.find(socket);
			if (!task) {
				Error << "illegal socket: " << socket;
				easynet->closeSocket(socket);
			}
			else {
	            CommonMessage* rawmsg = CastCommonMessage(easynet, netmsg);
				if (!sClientTaskManager.msgParser(easynet, socket, rawmsg)) {
					easynet->closeSocket(socket);
				}
			}
			easynet->releaseMessage(netmsg);
		}
	}

	bool GatewayService::sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		ClientTask* task = sClientTaskManager.find(socket);
		CHECK_RETURN(task, false, "not found ClientTask: %d", socket);
		return task->sendMessage(entityid, msgid, message);
	}
	
	INITIALIZE_INSTANCE(GatewayService);
}


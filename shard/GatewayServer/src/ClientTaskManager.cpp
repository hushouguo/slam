/*
 * \file: ClientTaskManager.cpp
 * \brief: Created by hushouguo at 13:43:15 Apr 30 2019
 */

#include "common.h"
#include "ClientTask.h"
#include "ClientTaskManager.h"
#include "SceneClient.h"
#include "SceneClientManager.h"
#include "GatewayPlayer.h"
#include "GatewayPlayerManager.h"

DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	bool ClientTaskManager::init(const char* address, int port, size_t number) {
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
					[this]() {
					});
			SOCKET fd = easynet->createServer(address, port);
			CHECK_RETURN(fd != EASYNET_ILLEGAL_SOCKET, false, "createServer:(%s:%d) failure", address, port);
			this->_easynets.push_back(easynet);
		}
		Debug << "ClientTaskManager create " << number << " Easynet";
		return true;
	}

	void ClientTaskManager::stop() {
		//
		// cleanup ClientTask
		struct ClientTaskCallback : public Callback<ClientTask> {
			bool invoke(ClientTask* entry) {
				SafeDelete(entry);
				return true;
			}
		}eee;
		this->traverse(eee);
		this->clear();

		//
		// cleanup Easynet
		for (auto& easynet : this->_easynets) {
			SafeDelete(easynet);
		}
		this->_easynets.clear();
	}

	void ClientTaskManager::run() {
		CHECK_RETURN(!this->_easynets.empty(), void(0), "ClientTaskManager not initiated");
		for (auto& easynet : this->_easynets) {
			this->run(easynet);
		}
	}	 

	void ClientTaskManager::run(Easynet* easynet) {
		//
		// handle connection state
		while (!sConfig.halt) {
			bool state = false;
			SOCKET socket = easynet->getSocketState(&state);
			if (socket == EASYNET_ILLEGAL_SOCKET) {
				break;
			}
			if (state) {
				ClientTask* task = new ClientTask(easynet, socket);
				if (!this->add(task)) {
					SafeDelete(task);
				}
				Debug << "new client task arrived: " << socket;
			}
			else {
				Alarm << "lost client task: " << socket;
				ClientTask* task = this->find(socket);
				if (task) {
					sGatewayPlayerManager.lostClient(task);					
					this->remove(task);
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
			assert(socket != EASYNET_ILLEGAL_SOCKET);
			ClientTask* task = this->find(socket);
			if (!task) {
				Error << "illegal socket: " << socket;
				easynet->closeSocket(socket);
			}
			else {
	            CommonMessage* rawmsg = CastCommonMessage(easynet, netmsg);
				if (!this->msgParser(easynet, socket, rawmsg)) {
					easynet->closeSocket(socket);
				}
			}
			easynet->releaseMessage(netmsg);
		}
	}

	bool ClientTaskManager::sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		ClientTask* task = this->find(socket);
		CHECK_RETURN(task, false, "not found ClientTask: %d", socket);
		return task->sendMessage(entityid, msgid, message);
	}
	
	bool ClientTaskManager::msgParser(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg) {
		return DISPATCH_MESSAGE(easynet, socket, rawmsg);
	}

	INITIALIZE_INSTANCE(ClientTaskManager);
}


//Note: ON_MSG(MSGID, STRUCTURE) 
// 	 ON_MSG(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
//

ON_MSG(MSGID_HEARTBEAT, Heartbeat) {
#if 0
	Heartbeat res;
	//res.set_systime(sTime.milliseconds());
	res.set_systime(msg->systime());
	task->sendMessage(fd, MSGID_HEARTBEAT, &res, 0);
#endif	
	sTime.now();
	Debug("receive heartbeat: %ld, systime: %ld", msg->milliseconds(), sTime.milliseconds());
}



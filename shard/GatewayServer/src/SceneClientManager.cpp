/*
 * \file: SceneClientManager.cpp
 * \brief: Created by hushouguo at 13:43:52 Apr 30 2019
 */

#include "common.h"
#include "ClientTask.h"
#include "ClientTaskManager.h"
#include "SceneClient.h"
#include "SceneClientManager.h"
#include "GatewayPlayer.h"
#include "GatewayPlayerManager.h"
#include "CentralClient.h"
#include "MainProcess.h"

DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	bool SceneClientManager::init() {	
		assert(this->_easynet == nullptr);
		this->_easynet = Easynet::createInstance(
			[](const void* buffer, size_t len) -> int {
				CommonMessage* msg = (CommonMessage*) buffer;
				return len < sizeof(CommonMessage) || len < msg->len ? 0 : msg->len;
				}, 
			[]() {
				sMainProcess.wakeup();
				});
		assert(this->_easynet);				
		ServerRetrieveRequest request;
		request.set_svrtype(SERVER_TYPE_SCENE);
		return sCentralClient.sendMessage(0, SMSGID_SERVER_RETRIEVE_REQ, &request);
	}

	bool SceneClientManager::init(u64 sceneServerid) {
		CHECK_RETURN(this->findSceneClient(sceneServerid) == nullptr, false, "duplicate register sceneServer: %ld", sceneServerid);
		std::pair<std::string, int> pair = splitNetworkEndpoint(sceneServerid);
		SOCKET socket = this->_easynet->createClient(pair.first.c_str(), pair.second, 0);
		CHECK_RETURN(socket != EASYNET_ILLEGAL_SOCKET, false, "illegal address: (%s:%d)", pair.first.c_str(), pair.second);
		SceneClient* sceneClient = new SceneClient(this->_easynet, socket, sceneServerid);
		Debug("connect to SceneServer: %s:%d", pair.first.c_str(), pair.second);
		return this->add(sceneClient);
	}

	SceneClient* SceneClientManager::findSceneClient(u64 sceneServerid) {
		SceneClient* sceneClient = nullptr;
		this->traverse([sceneServerid, &sceneClient](SceneClient* entry)->bool{
				if (entry->sceneServerid() == sceneServerid) {
					sceneClient = entry;
					return false;
				}
				return true;
		});
		return sceneClient;
	}

	void SceneClientManager::stop() {
		//
		// cleanup SceneClient
		this->traverse([](SceneClient* entry)->bool{
			SafeDelete(entry);
			return true;
		});
		this->clear();

		//
		// cleanup Easynet
		SafeDelete(this->_easynet);
	}

	void SceneClientManager::run() {
		//
		// handle connection state
		while (!sConfig.halt) {
			bool state = false;
			SOCKET socket = this->_easynet->getSocketState(&state);
			if (socket == EASYNET_ILLEGAL_SOCKET) {
				break;
			}
			if (!state) {
				Alarm << "lost SceneClient: " << socket;
				SceneClient* client = this->find(socket);
				if (client) {
					sGatewayPlayerManager.lostSceneServer(client);					
					this->remove(client);
					SafeDelete(client);
				}
			}
		}

		//
		// handle message
		while (!sConfig.halt) {
			SOCKET socket = EASYNET_ILLEGAL_SOCKET;
			const void* netmsg = this->_easynet->receiveMessage(&socket);
			if (!netmsg) {
				break;
			}
			assert(socket != EASYNET_ILLEGAL_SOCKET);
			SceneClient* client = this->find(socket);
			if (!client) {
				Error << "illegal socket: " << socket;
				this->_easynet->closeSocket(socket);
			}
			else {
	            CommonMessage* rawmsg = CastCommonMessage(this->_easynet, netmsg);
	            if (!DISPATCH_MESSAGE(this->_easynet, socket, rawmsg)) {
					this->_easynet->closeSocket(socket);
				}
			}
			this->_easynet->releaseMessage(netmsg);
		}
	}

	bool SceneClientManager::sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		SceneClient* client = this->find(socket);
		CHECK_RETURN(client, false, "not found SceneClient: %d", socket);
		return client->sendMessage(entityid, msgid, message);
	}

	INITIALIZE_INSTANCE(SceneClientManager);
}

using namespace slam;

//Note: ON_MSG(MSGID, STRUCTURE) 
// 	 ON_MSG(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
//

#if 0
ON_MSG(MSGID_HEARTBEAT, Heartbeat) {
	Heartbeat res;
	//res.set_systime(sTime.milliseconds());
	res.set_systime(msg->systime());
	task->sendMessage(fd, MSGID_HEARTBEAT, &res, 0);
	//log_trace("receive heartbeat: %ld, systime: %ld", msg->systime(), res.systime());
}
#endif	


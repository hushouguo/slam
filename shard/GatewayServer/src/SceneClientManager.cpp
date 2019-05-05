/*
 * \file: SceneClientManager.cpp
 * \brief: Created by hushouguo at 13:43:52 Apr 30 2019
 */

#include "common.h"
#include "SceneClient.h"
#include "SceneClientManager.h"


DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	SceneClientManager::SceneClientManager() {
		this->_easynet = Easynet::createInstance(
			[](const void* buffer, size_t len) -> int {
				CommonMessage* msg = (CommonMessage*) buffer;
				return len < sizeof(CommonMessage) || len < msg->len ? 0 : msg->len;
				}, 
			[this]() {
				});
	}
	
	bool SceneClientManager::init() {	
#if false	
		NetData::ServerRetrieveRequest request;
		request.set_stype(SERVER_TYPE_SCENE);
		return sCentralClient.sendMessage(MSGID_SERVER_RETRIEVE_REQ, &request, 0);
#endif	

		return false;
	}

	bool SceneClientManager::init(u64 sceneServerid) {
		//NOTE: check sceneServerid if exists
		std::pair<std::string, int> pair = splitNetworkEndpoint(sceneServerid);
		SOCKET socket = this->_easynet->createClient(pair.first.c_str(), pair.second, 0);
		CHECK_RETURN(socket != EASYNET_ILLEGAL_SOCKET, false, "illegal address: (%s:%d)", pair.first.c_str(), pair.second);
		SceneClient* sceneClient = new SceneClient(this->_easynet, socket);
		Debug("connect to SceneServer: %s:%d", pair.first.c_str(), pair.second);		
		return this->add(sceneClient);
	}

	void SceneClientManager::stop() {
		//
		// cleanup SceneClient
		struct SceneClientCallback : public Callback<SceneClient> {
			bool invoke(SceneClient* entry) {
				SafeDelete(entry);
				return true;
			}
		}eee;
		this->traverse(eee);
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
				SceneClient* client = this->find(socket);
				if (client) {
					this->remove(client);
					SafeDelete(client);
				}
				Alarm << "lost SceneClient: " << socket;
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
			
			size_t len = 0;
			const void* payload = this->_easynet->getMessageContent(netmsg, &len);
			assert(len >= sizeof(CommonMessage));
			CommonMessage* rawmsg = (CommonMessage*) payload;
			assert(rawmsg->len >= sizeof(CommonMessage));
			if (!this->msgParser(socket, rawmsg)) {
				this->_easynet->closeSocket(socket);
			}
			this->_easynet->releaseMessage(netmsg);
		}
	}

	bool SceneClientManager::sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		SceneClient* client = this->find(socket);
		CHECK_RETURN(client, false, "not found SceneClient: %d", socket);
		return client->sendMessage(entityid, msgid, message);
	}

	bool SceneClientManager::msgParser(SOCKET socket, CommonMessage* rawmsg) {
		return DISPATCH_MESSAGE(this->_easynet, socket, rawmsg);
	}

	INITIALIZE_INSTANCE(SceneClientManager);
}

//Note: ON_MSG(MSGID, STRUCTURE) 
// 	 ON_MSG(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
//

#if false
ON_MSG(MSGID_HEARTBEAT, Heartbeat) {
	Heartbeat res;
	//res.set_systime(sTime.milliseconds());
	res.set_systime(msg->systime());
	task->sendMessage(fd, MSGID_HEARTBEAT, &res, 0);
	//log_trace("receive heartbeat: %ld, systime: %ld", msg->systime(), res.systime());
}
#endif	


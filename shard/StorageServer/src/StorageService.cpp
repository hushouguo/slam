/*
 * \file: StorageService.cpp
 * \brief: Created by hushouguo at 11:24:37 May 06 2019
 */

#include "common.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"

DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	bool StorageService::init(const char* address, int port) {
		assert(this->_easynet == nullptr);
		this->_easynet = Easynet::createInstance(
				[](const void* buffer, size_t len) -> int {
					CommonMessage* msg = (CommonMessage*) buffer;
					return len < sizeof(CommonMessage) || len < msg->len ? 0 : msg->len;
				}, 
				[]() {
				});
		SOCKET socket = this->_easynet->createServer(address, port);
		CHECK_RETURN(!ILLEGAL_SOCKET(socket), false, "createServer:(%s:%d) failure", address, port);
		Debug << "StorageService listening on " << address << ":" << port;
		return true;
	}

	void StorageService::stop() {
		SafeDelete(this->_easynet);
	}

	void StorageService::run() {
		CHECK_RETURN(this->_easynet, void(0), "StorageService not initiated");

		//
		// handle connection state
        while (!sConfig.halt) {
            bool state = false;
            SOCKET socket = this->_easynet->getSocketState(&state);
            if (socket == EASYNET_ILLEGAL_SOCKET) {
                break;
            }
            //assert(socket == this->id);
            if (!state) {
            	CentralTask* centralTask = sCentralTaskManager.find(socket);
            	if (centralTask) {
					Error << "lost CentralTask: " << socket;
            		sCentralTaskManager.remove(centralTask);
            		SafeDelete(centralTask);
            	}
            	
            	SceneTask* sceneTask = sSceneTaskManager.find(socket);
            	if (sceneTask) {
					Error << "lost SceneTask: " << socket;
            		sSceneTaskManager.remove(sceneTask);
            		SafeDelete(sceneTask);
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
            assert(!ILLEGAL_SOCKET(socket));
            
            CommonMessage* rawmsg = CastCommonMessage(this->_easynet, netmsg);
            if (sCentralTaskManager.find(socket) != nullptr) {
            	if (!sCentralTaskManager.msgParser(this->_easynet, socket, rawmsg)) {
            		this->_easynet->closeSocket(socket);
            	}
            }
            else if (sSceneTaskManager.find(socket) != nullptr) {
            	if (!sSceneTaskManager.msgParser(this->_easynet, socket, rawmsg)) {
            		this->_easynet->closeSocket(socket);
            	}
            }
            else {
            	if (!DISPATCH_MESSAGE(this->_easynet, socket, rawmsg)) {
					this->_easynet->closeSocket(socket);
            	}
            }
            this->_easynet->releaseMessage(netmsg);
        }
	}

	INITIALIZE_INSTANCE(StorageService);
}

using namespace slam;

//Note: ON_MSG(MSGID, STRUCTURE) 
// 	 ON_MSG(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
//

ON_MSG(SMSGID_SERVER_REGISTER_REQ, ServerRegisterRequest) {
	switch (msg->svrtype()) {
		case SERVER_TYPE_CENTRAL: {
				CentralTask* centralTask = new CentralTask(easynet, socket);
				bool rc = sCentralTaskManager.add(centralTask);
				Debug("register CentralServer: %d, %d, %s", msg->shard(), msg->port(), rc ? "OK" : "failure");

				ServerRegisterResponse response;
				response.set_rc(rc);
				response.set_svrtype(SERVER_TYPE_STORAGE);
				SendMessage(easynet, socket, 0, SMSGID_SERVER_REGISTER_REP, &response);
			}
			break;

		case SERVER_TYPE_SCENE: {
				SceneTask* sceneTask = new SceneTask(easynet, socket);
				bool rc = sSceneTaskManager.add(sceneTask);
				Debug("register SceneServer: %d, %d, %s", msg->shard(), msg->port(), rc ? "OK" : "failure");

				ServerRegisterResponse response;
				response.set_rc(rc);
				response.set_svrtype(SERVER_TYPE_STORAGE);
				SendMessage(easynet, socket, 0, SMSGID_SERVER_REGISTER_REP, &response);
			}
			break;

		default:
			CHECK_BREAK(false, "illegal register server type: %d", msg->svrtype());
	}
}

ON_MSG(SMSGID_SERVER_RETRIEVE_REQ, ServerRetrieveRequest) {
}


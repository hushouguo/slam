/*
 * \file: SceneTaskManager.cpp
 * \brief: Created by hushouguo at 09:42:09 May 05 2019
 */

DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	SceneTaskManager::SceneTaskManager() {
		this->_easynet = Easynet::createInstance(
			[](const void* buffer, size_t len) -> int {
				CommonMessage* msg = (CommonMessage*) buffer;
				return len < sizeof(CommonMessage) || len < msg->len ? 0 : msg->len;
				}, 
			[]() {
				sMainProcess.wakeup();
				});
	}
	
	bool SceneTaskManager::init() {	
		ServerRetrieveRequest request;
		request.set_svrtype(SERVER_TYPE_SCENE);
		return sCentralClient.sendMessage(0, SMSGID_SERVER_RETRIEVE_REQ, &request);
	}

	bool SceneTaskManager::init(u64 sceneServerid) {
		CHECK_RETURN(this->findSceneTask(sceneServerid) == nullptr, false, "duplicate register sceneServer: %ld", sceneServerid);
		std::pair<std::string, int> pair = splitNetworkEndpoint(sceneServerid);
		SOCKET socket = this->_easynet->createClient(pair.first.c_str(), pair.second, 0);
		CHECK_RETURN(socket != EASYNET_ILLEGAL_SOCKET, false, "illegal address: (%s:%d)", pair.first.c_str(), pair.second);
		SceneTask* sceneClient = new SceneTask(this->_easynet, socket, sceneServerid);
		Debug("connect to SceneServer: %s:%d", pair.first.c_str(), pair.second);
		return this->add(sceneClient);
	}

	SceneTask* SceneTaskManager::findSceneTask(u64 sceneServerid) {
		struct SceneTaskCallback : public Callback<SceneTask> {
			SceneTask* sceneClient = nullptr;
			u64 sceneServerid = 0;
			SceneTaskCallback(u64 _sceneServerid) : sceneServerid(_sceneServerid) {}
			bool invoke(SceneTask* entry) {
				if (entry->sceneServerid() == this->sceneServerid) {
					this->sceneClient = entry;
					return false;
				}
				return true;
			}
		}eee(sceneServerid);
		this->traverse(eee);
		return eee.sceneClient;
	}

	void SceneTaskManager::stop() {
		//
		// cleanup SceneTask
		struct SceneTaskCallback : public Callback<SceneTask> {
			bool invoke(SceneTask* entry) {
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

	void SceneTaskManager::run() {
		//
		// handle connection state
		while (!sConfig.halt) {
			bool state = false;
			SOCKET socket = this->_easynet->getSocketState(&state);
			if (socket == EASYNET_ILLEGAL_SOCKET) {
				break;
			}
			if (!state) {
				Alarm << "lost SceneTask: " << socket;
				SceneTask* client = this->find(socket);
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
			SceneTask* client = this->find(socket);
			if (!client) {
				Error << "illegal socket: " << socket;
				this->_easynet->closeSocket(socket);
			}
			else {
	            CommonMessage* rawmsg = CastCommonMessage(this->_easynet, netmsg);
				if (!this->msgParser(socket, rawmsg)) {
					this->_easynet->closeSocket(socket);
				}
			}
			this->_easynet->releaseMessage(netmsg);
		}
	}

	bool SceneTaskManager::sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		SceneTask* client = this->find(socket);
		CHECK_RETURN(client, false, "not found SceneTask: %d", socket);
		return client->sendMessage(entityid, msgid, message);
	}

	bool SceneTaskManager::msgParser(SOCKET socket, CommonMessage* rawmsg) {
		return DISPATCH_MESSAGE(this->_easynet, socket, rawmsg);
	}

	INITIALIZE_INSTANCE(SceneTaskManager);
}

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



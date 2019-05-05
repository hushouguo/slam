/*
 * \file: CentralTaskManager.cpp
 * \brief: Created by hushouguo at 13:43:15 Apr 30 2019
 */

DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	bool CentralTaskManager::init(const char* address, int port, size_t number) {
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
		Debug << "CentralTaskManager create " << number << " Easynet";
		return true;
	}

	void CentralTaskManager::stop() {
		for (auto& easynet : this->_easynets) {			
			SafeDelete(easynet);			
		}
		this->_easynets.clear();
		//TODO: cleanup client tasks
	}

	void CentralTaskManager::run() {
		CHECK_RETURN(!this->_easynets.empty(), void(0), "CentralTaskManager not initiated");
		for (auto& easynet : this->_easynets) {
			this->run(easynet);
		}
	}	 

	void CentralTaskManager::run(Easynet* easynet) {
		//
		// handle connection state
		while (!sConfig.halt) {
			bool state = false;
			SOCKET socket = easynet->getSocketState(&state);
			if (socket == EASYNET_ILLEGAL_SOCKET) {
				break;
			}
			if (state) {
				CentralTask* task = new CentralTask(easynet, socket);
				if (!this->add(task)) {
					SafeDelete(task);
				}
				Debug << "new client task arrived: " << socket;
			}
			else {
				CentralTask* task = this->find(socket);
				if (task) {
					this->remove(socket);
					SafeDelete(task);
				}
				Alarm << "lost client task: " << socket;
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
			
			size_t len = 0;
			const void* payload = easynet->getMessageContent(netmsg, &len);
			assert(len >= sizeof(CommonMessage));
			CommonMessage* rawmsg = (CommonMessage*) payload;
			assert(rawmsg->len >= sizeof(CommonMessage));
			if (!this->msgParser(easynet, socket, rawmsg)) {
				this->stop();
			}
			easynet->releaseMessage(netmsg);
		}
	}

	bool CentralTaskManager::sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		//TODO:
		return false;
	}
	
	bool CentralTaskManager::msgParser(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg) {
		return DISPATCH_MESSAGE(easynet, socket, rawmsg);
	}

	INITIALIZE_INSTANCE(CentralTaskManager);
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



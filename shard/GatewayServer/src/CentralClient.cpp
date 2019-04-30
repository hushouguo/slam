/*
 * \file: CentralClient.cpp
 * \brief: Created by hushouguo at 13:43:33 Apr 30 2019
 */

DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	bool CentralClient::init(const char* address, int port) {
		this->_easynet = Easynet::createInstance(
			[](const void* buffer, size_t len) -> int {
				CommonMessage* msg = (CommonMessage*) buffer;
				return len < sizeof(CommonMessage) || len < msg->len ? 0 : msg->len;
				}, 
			[this]() {
				});

		this->_fd_centralclient = this->_easynet->createClient(address, port, 0);
		if (this->_fd_centralclient == EASYNET_ILLEGAL_SOCKET) {
			this->stop();
			return false;
		}

		return true;
	}

	void CentralClient::stop() {
		SafeDelete(this->_easynet);
		this->_fd_centralclient = EASYNET_ILLEGAL_SOCKET;
		sConfig.syshalt(0);
	}

	void CentralClient::run() {
		CHECK_RETURN(this->_easynet && this->_fd_centralclient != EASYNET_ILLEGAL_SOCKET, 
			void(0), "centralClient not initiated");

		//
		// handle connection state
        while (!sConfig.halt) {
            bool state = false;
            SOCKET socket = this->_easynet->getSocketState(&state);
            if (socket == EASYNET_ILLEGAL_SOCKET) {
                break;
            }
            assert(socket == this->_fd_centralclient);
            if (state) {
				Debug << "CentralClient successful connection";
            }
            else {
            	Error << "lost connection with CentralServer";
            	this->stop();
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
            assert(socket == this->_fd_centralclient);
            
            size_t len = 0;
            const void* payload = this->_easynet->getMessageContent(netmsg, &len);
            assert(len >= sizeof(CommonMessage));
            CommonMessage* rawmsg = (CommonMessage*) payload;
            assert(rawmsg->len >= sizeof(CommonMessage));
            if (!this->msgParser(rawmsg)) {
            	this->stop();
            }
            this->_easynet->releaseMessage(netmsg);
        }
	}	

	bool CentralClient::sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		assert(message);
		assert(this->_easynet);
		
        //   
        // allocate new CommonMessage
        size_t byteSize = message->ByteSize();
        const void* netmsg = this->_easynet->allocateMessage(byteSize + sizeof(CommonMessage));
        size_t len = 0; 
        CommonMessage* msg = (CommonMessage*) this->_easynet->getMessageContent(netmsg, &len); 
        assert(len == byteSize + sizeof(CommonMessage));
     
        //   
        // serialize protobuf::Message to ServiceMessage
        bool rc = message->SerializeToArray(msg->payload, byteSize);
        if (!rc) {
            this->_easynet->releaseMessage(netmsg);
            Error("Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
            return false;
        }

        //   
        // send CommonMessage to network
        msg->len = len;
        msg->entityid = entityid;
        msg->msgid = msgid;
        msg->flags = 0;
        return this->_easynet->sendMessage(this->_fd_centralclient, netmsg);
	}

	bool CentralClient::msgParser(CommonMessage* rawmsg) {
		return DISPATCH_MESSAGE(this->_easynet, this->_fd_centralclient, rawmsg);
	}
	
	INITIALIZE_INSTANCE(CentralClient);
}


//Note: ON_MSG(MSGID, STRUCTURE) 
//		ON_MSG(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
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


/*
 * \file: StorageService.cpp
 * \brief: Created by hushouguo at 11:24:37 May 06 2019
 */

BEGIN_NAMESPACE_SLAM {
	bool StorageService::init(const char* address, int port) {
		if (this->_easynet) { return true; }
		this->_easynet = Easynet::createInstance(
				[](const void* buffer, size_t len) -> int {
					CommonMessage* msg = (CommonMessage*) buffer;
					return len < sizeof(CommonMessage) || len < msg->len ? 0 : msg->len;
				}, 
				[]() {
				});
		this->id = this->_easynet->createServer(address, port);
		CHECK_RETURN(this->id != EASYNET_ILLEGAL_SOCKET, false, "createServer:(%s:%d) failure", address, port);
		return true;
	}

	void StorageService::stop() {
		SafeDelete(this->_easynet);
		this->id = EASYNET_ILLEGAL_SOCKET;
	}

	void StorageService::run() {
		CHECK_RETURN(this->_easynet && this->id != EASYNET_ILLEGAL_SOCKET, void(0), "StorageService not initiated");

		//
		// handle connection state
        while (!sConfig.halt) {
            bool state = false;
            SOCKET socket = this->_easynet->getSocketState(&state);
            if (socket == EASYNET_ILLEGAL_SOCKET) {
                break;
            }
            assert(socket == this->id);
            if (state) {
				Debug << "CentralClient successful connect";
            }
            else {
            	Error << "lost connection with CentralServer";
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
            assert(socket == this->id);
#if false
            CommonMessage* rawmsg = CastCommonMessage(this->_easynet, netmsg);
            if (!this->msgParser(rawmsg)) {
            	this->stop();
            }
#endif			
            this->_easynet->releaseMessage(netmsg);
        }
	}

	INITIALIZE_INSTANCE(StorageServiceManager);
}

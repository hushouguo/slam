/*
 * \file: CentralClient.cpp
 * \brief: Created by hushouguo at 13:43:33 Apr 30 2019
 */

#include "common.h"
#include "CentralClient.h"
#include "SceneClient.h"
#include "SceneClientManager.h"

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

		this->id = this->_easynet->createClient(address, port, 0);
		if (this->id == EASYNET_ILLEGAL_SOCKET) {
			this->stop();
			return false;
		}

		return true;
	}

	void CentralClient::stop() {
		SafeDelete(this->_easynet);
		this->id = EASYNET_ILLEGAL_SOCKET;
		sConfig.syshalt(0);
	}

	void CentralClient::run() {
		CHECK_RETURN(this->_easynet && this->id != EASYNET_ILLEGAL_SOCKET, void(0), "centralClient not initiated");

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
            assert(socket == this->id);

            CommonMessage* rawmsg = CastCommonMessage(this->_easynet, netmsg);
            if (!this->msgParser(rawmsg)) {
            	this->stop();
            }
            this->_easynet->releaseMessage(netmsg);
        }
	}	

	bool CentralClient::sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		return SendMessage(this->_easynet, this->id, entityid, msgid, message);
	}

	bool CentralClient::msgParser(CommonMessage* rawmsg) {
		return DISPATCH_MESSAGE(this->_easynet, this->id, rawmsg);
	}
	
	INITIALIZE_INSTANCE(CentralClient);
}


//Note: ON_MSG(MSGID, STRUCTURE) 
//		ON_MSG(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
//

//RegisterServerResponse
ON_MSG(SMSGID_SERVER_REGISTER_REP, ServerRegisterResponse)
{
	switch (msg->svrtype()) {
		case SERVER_TYPE_SCENE:
			if (msg->rc()) {
				Debug << "register to SceneServer OK";
			}
			else {
				Panic << "register to SceneServer failure, try to reboot process";
			}
			break;

		case SERVER_TYPE_CENTRAL:
			if (msg->rc()) {
				Debug << "register to CentralServer OK";
			}
			else {
				Panic << "register to CentralServer failure, try to reboot process";
			}
			break;

		default:
			CHECK_BREAK(false, "illegal server type: %d", msg->svrtype());
	}
}

//ServerRetrieveResponse
ON_MSG(SMSGID_SERVER_RETRIEVE_REP, ServerRetrieveResponse)
{
	switch (msg->svrtype()) {
		case SERVER_TYPE_SCENE:
			for (int n = 0; n < msg->servers_size(); ++n) {
				sSceneClientManager.init(msg->servers(n));
			}
			break;

		default:
			CHECK_BREAK(false, "illegal server type: %d", msg->svrtype());
	}
}


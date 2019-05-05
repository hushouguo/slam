/*
 * \file: CommonMessage.cpp
 * \brief: Created by hushouguo at 13:34:50 May 05 2019
 */

#include "common.h"

BEGIN_NAMESPACE_SLAM {
	bool SendMessage(Easynet* easynet, SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		assert(message);
		assert(easynet);
		
        //   
        // allocate new CommonMessage
        size_t byteSize = message->ByteSize();
        const void* netmsg = easynet->allocateMessage(byteSize + sizeof(CommonMessage));
        size_t len = 0; 
        CommonMessage* msg = (CommonMessage*) easynet->getMessageContent(netmsg, &len); 
        assert(len == byteSize + sizeof(CommonMessage));
     
        //   
        // serialize protobuf::Message to ServiceMessage
        bool rc = message->SerializeToArray(msg->payload, byteSize);
		if (!rc) {
			easynet->releaseMessage(netmsg);
			Error("Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
			return false;
		}

        //   
        // send CommonMessage to network
        msg->len = len;
        msg->entityid = entityid;
        msg->msgid = msgid;
        msg->flags = 0;
        return easynet->sendMessage(socket, netmsg);
	}
}

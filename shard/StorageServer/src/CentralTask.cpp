/*
 * \file: CentralTask.cpp
 * \brief: Created by hushouguo at 13:43:06 Apr 30 2019
 */

BEGIN_NAMESPACE_SLAM {
	CentralTask::CentralTask(Easynet* easynet, SOCKET socket) : Entry<SOCKET>(socket) {
		this->_entityid = 0;
		this->_easynet = easynet;
	}
	
	bool CentralTask::sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
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
        return this->_easynet->sendMessage(this->id, netmsg);
	}
}


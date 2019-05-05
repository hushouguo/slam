/*
 * \file: ClientTask.cpp
 * \brief: Created by hushouguo at 13:43:06 Apr 30 2019
 */

#include "common/common.h"
#include "ClientTask.h"

BEGIN_NAMESPACE_SLAM {
	ClientTask::ClientTask(Easynet* easynet, SOCKET socket) : Entry<SOCKET>(socket) {
		this->_easynet = easynet;
	}
	
	bool ClientTask::sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		return SendMessage(this->_easynet, this->id, entityid, msgid, message);
	}
}


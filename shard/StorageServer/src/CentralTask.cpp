/*
 * \file: CentralTask.cpp
 * \brief: Created by hushouguo at 13:43:06 Apr 30 2019
 */

BEGIN_NAMESPACE_SLAM {
	CentralTask::CentralTask(Easynet* easynet, SOCKET socket) : Entry<SOCKET>(socket) {
		this->_easynet = easynet;
	}
	
	bool CentralTask::sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		return SendMessage(this->_easynet, this->id, entityid, msgid, message);
	}
}


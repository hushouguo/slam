/*
 * \file: SceneClient.cpp
 * \brief: Created by hushouguo at 13:43:43 Apr 30 2019
 */

#include "common.h"
#include "SceneClient.h"


BEGIN_NAMESPACE_SLAM {
	SceneClient::SceneClient(Easynet* easynet, SOCKET socket, u64 sceneServerid) 
		: Entry<SOCKET>(socket) 
		, _sceneServerid(sceneServerid)
		, _easynet(easynet)
	{
	}
	
	bool SceneClient::sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		return SendMessage(this->_easynet, this->id, entityid, msgid, message);
	}
}


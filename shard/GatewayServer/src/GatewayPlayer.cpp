/*
 * \file: GatewayPlayer.cpp
 * \brief: Created by hushouguo at 13:44:48 Apr 30 2019
 */

#include "common/common.h"
#include "ClientTask.h"
#include "ClientTaskManager.h"
#include "CentralClient.h"
#include "SceneClient.h"
#include "SceneClientManager.h"
#include "GatewayPlayer.h"
#include "GatewayPlayerManager.h"

BEGIN_NAMESPACE_SLAM {
	GatewayPlayer::GatewayPlayer(u64 playerid, ClientTask* clientTask, SceneClient* sceneClient) 
		: Entry<u64>(playerid)
		, _clientTask(clientTask)
		, _sceneClient(sceneClient)
	{
	}
	
	bool GatewayPlayer::sendMessageToCentral(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		return sCentralClient.sendMessage(entityid, msgid, message);
	}
	
	bool GatewayPlayer::sendMessageToScene(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		CHECK_RETURN(this->_sceneClient != nullptr, false, "msgid:%d, sceneClient unavailable", msgid);
		return this->_sceneClient->sendMessage(entityid, msgid, message);
	}
	
	bool GatewayPlayer::sendMessageToClient(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		CHECK_RETURN(this->_clientTask != nullptr, false, "msgid:%d, clientTask unavailable", msgid);
		return this->_clientTask->sendMessage(entityid, msgid, message);
	}
}


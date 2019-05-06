/*
 * \file: ClientTaskManager.cpp
 * \brief: Created by hushouguo at 13:43:15 Apr 30 2019
 */

#include "common.h"
#include "ClientTask.h"
#include "ClientTaskManager.h"
#include "SceneClient.h"
#include "SceneClientManager.h"
#include "GatewayPlayer.h"
#include "GatewayPlayerManager.h"
#include "MainProcess.h"

DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	bool ClientTaskManager::sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		ClientTask* task = this->find(socket);
		CHECK_RETURN(task, false, "not found ClientTask: %d", socket);
		return task->sendMessage(entityid, msgid, message);
	}
	
	bool ClientTaskManager::msgParser(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg) {
		return DISPATCH_MESSAGE(easynet, socket, rawmsg);
	}

	INITIALIZE_INSTANCE(ClientTaskManager);
}


//Note: ON_MSG(MSGID, STRUCTURE) 
// 	 ON_MSG(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
//

ON_MSG(MSGID_HEARTBEAT, Heartbeat) {
#if 0
	Heartbeat res;
	//res.set_systime(sTime.milliseconds());
	res.set_systime(msg->systime());
	task->sendMessage(fd, MSGID_HEARTBEAT, &res, 0);
#endif	
	sTime.now();
	Debug("receive heartbeat: %ld, systime: %ld", msg->milliseconds(), sTime.milliseconds());
}



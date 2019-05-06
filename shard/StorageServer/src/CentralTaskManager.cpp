/*
 * \file: CentralTaskManager.cpp
 * \brief: Created by hushouguo at 13:43:15 Apr 30 2019
 */

#include "common.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"

DECLARE_MESSAGE();

BEGIN_NAMESPACE_SLAM {
	bool CentralTaskManager::msgParser(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg) {
		return DISPATCH_MESSAGE(easynet, socket, rawmsg);
	}

	INITIALIZE_INSTANCE(CentralTaskManager);
}

using namespace slam;

//Note: ON_MSG(MSGID, STRUCTURE) 
// 	 ON_MSG(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
//

ON_MSG(SMSGID_STORAGE_SERIALIZE_REQ, StorageSerializeRequest) {
	sStorageProcessManager.serialize(easynet, socket, msg);
}

ON_MSG(SMSGID_STORAGE_UNSERIALIZE_REQ, StorageUnserializeRequest) {
	sStorageProcessManager.unserialize(easynet, socket, msg);
}


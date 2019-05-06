/*
 * \file: SceneTask.cpp
 * \brief: Created by hushouguo at 09:42:01 May 05 2019
 */

#include "common.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"

BEGIN_NAMESPACE_SLAM {
	SceneTask::SceneTask(Easynet* easynet, SOCKET socket) : Entry<SOCKET>(socket) {
		this->_easynet = easynet;
	}
	
	bool SceneTask::sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		return SendMessage(this->_easynet, this->id, entityid, msgid, message);
	}
}

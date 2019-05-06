/*
 * \file: CentralTask.cpp
 * \brief: Created by hushouguo at 13:43:06 Apr 30 2019
 */

#include "common.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"

BEGIN_NAMESPACE_SLAM {
	CentralTask::CentralTask(Easynet* easynet, SOCKET socket) : Entry<SOCKET>(socket) {
		this->_easynet = easynet;
	}
	
	bool CentralTask::sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message) {
		return SendMessage(this->_easynet, this->id, entityid, msgid, message);
	}
}


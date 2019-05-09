/*
 * \file: StorageProcess.cpp
 * \brief: Created by hushouguo at 09:49:54 May 05 2019
 */

#include "common.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"
#include "StorageEntity.h"
#include "StorageEntityManager.h"
#include "MessageStatement.h"
#include "StorageHandler.h"
#include "StorageHandlerManager.h"
#include "StorageProcess.h"
#include "StorageProcessManager.h"

BEGIN_NAMESPACE_SLAM {
	StorageProcess::StorageProcess(int id) : Entry<int>(id) {
		
	}

	bool StorageProcess::init() {
		bool rc = this->_handlerManager.init();
		CHECK_RETURN(rc, false, "StorageHandlerManager init failure");
		return true;
	}
	
	void StorageProcess::serialize(Easynet* easynet, SOCKET socket, StorageSerializeRequest* request) {
		assert(request->entity().id() == 0 || (int)(request->entity().id() % sStorageProcessManager.size()) == this->id);
		
	}
	
	void StorageProcess::unserialize(Easynet* easynet, SOCKET socket, StorageUnserializeRequest* request) {
		assert((int)(request->entityid() % sStorageProcessManager.size()) == this->id);
	}

}


/*
 * \file: MainProcess.cpp
 * \brief: Created by hushouguo at 11:21:37 May 06 2019
 */

#include "common.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"

BEGIN_NAMESPACE_SLAM {
	 void MainProcess::run() {
		 while (!sConfig.halt) {
			 std::unique_lock<std::mutex> locker(this->_mtx);
			 this->_cond.wait(locker);

			 if (sConfig.reload) {
				 //NOTE: reload config & data files
				 Debug << "Need reload config files and data files";
			 }
			 sStorageService.run();
		 }
	 }

	 void MainProcess::wakeup() {
		 this->_cond.notify_all();
	 }

	 INITIALIZE_INSTANCE(MainProcess);
}


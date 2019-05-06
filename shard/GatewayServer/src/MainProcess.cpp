/*
 * \file: MainProcess.cpp
 * \brief: Created by hushouguo at 17:26:41 May 05 2019
 */

#include "common.h"
#include "ClientTask.h"
#include "ClientTaskManager.h"
#include "CentralClient.h"
#include "SceneClient.h"
#include "SceneClientManager.h"
#include "GatewayPlayer.h"
#include "GatewayPlayerManager.h"
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
			sClientTaskManager.run();
			//sCentralClient.run();
			sSceneClientManager.run();
		}
	}

	void MainProcess::wakeup() {
		this->_cond.notify_all();
	}

	INITIALIZE_INSTANCE(MainProcess);
}

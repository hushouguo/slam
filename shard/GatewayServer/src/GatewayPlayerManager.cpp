/*
 * \file: GatewayPlayerManager.cpp
 * \brief: Created by hushouguo at 13:44:56 Apr 30 2019
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
	void GatewayPlayerManager::loginUser(u64 playerid, u32 clientid, u32 sceneClientid) {
	}
	
	void GatewayPlayerManager::logoutUser(GatewayPlayer* gatewayPlayer, const char* reason, bool forward) {
	}

	void GatewayPlayerManager::lostClient(ClientTask* clientTask) {
	}
	
	void GatewayPlayerManager::lostSceneServer(SceneClient* sceneClient) {
	}
	
	INITIALIZE_INSTANCE(GatewayPlayerManager);
}


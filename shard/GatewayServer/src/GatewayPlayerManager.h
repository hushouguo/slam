/*
 * \file: GatewayPlayerManager.h
 * \brief: Created by hushouguo at 13:44:55 Apr 30 2019
 */
 
#ifndef __GATEWAYPLAYERMANAGER_H__
#define __GATEWAYPLAYERMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class GatewayPlayerManager : public Manager<GatewayPlayer> {
		public:
			void loginUser(u64 playerid, u32 clientid, u32 sceneClientid);
			void logoutUser(GatewayPlayer* gatewayPlayer, const char* reason, bool forward);
		
		public:
			void lostClient(ClientTask* clientTask);
			void lostSceneServer(SceneClient* sceneClient);
	};
}

#define sGatewayPlayerManager slam::Singleton<slam::GatewayPlayerManager>::getInstance()

#endif

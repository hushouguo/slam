/*
 * \file: GatewayPlayer.h
 * \brief: Created by hushouguo at 13:44:46 Apr 30 2019
 */
 
#ifndef __GATEWAYPLAYER_H__
#define __GATEWAYPLAYER_H__

BEGIN_NAMESPACE_SLAM {
	class GatewayPlayer : public Entry<u64> {
		public:
			GatewayPlayer(u64 playerid, ClientTask* clientTask, SceneClient* sceneClient);
			const char* getClassName() override { return "GatewayPlayer"; }

		public:
			inline ClientTask* clientTask() { return this->_clientTask; }
			inline void setClientTask(ClientTask* clientTask) { this->_clientTask = clientTask; }

			inline SceneClient* sceneClient() { return this->_sceneClient; }
			inline void setSceneClient(SceneClient* sceneClient) { this->_sceneClient = sceneClient; }

		public:
			bool sendMessageToCentral(u64 entityid, u32 msgid, const google::protobuf::Message* message);
			bool sendMessageToScene(u64 entityid, u32 msgid, const google::protobuf::Message* message);
			bool sendMessageToClient(u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			ClientTask* _clientTask = nullptr;
			SceneClient* _sceneClient = nullptr;
	};
}

#endif

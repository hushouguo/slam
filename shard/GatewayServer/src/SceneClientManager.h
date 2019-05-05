/*
 * \file: SceneClientManager.h
 * \brief: Created by hushouguo at 13:43:51 Apr 30 2019
 */
 
#ifndef __SCENECLIENTMANAGER_H__
#define __SCENECLIENTMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class SceneClientManager : public Manager<SceneClient> {
		public:
			SceneClientManager();
			
		public:
			bool init();
			bool init(u64 sceneServerid);
			void stop();
		
		public:	
			void run();
			bool msgParser(SOCKET socket, CommonMessage* rawmsg);

		public:
			bool sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			Easynet* _easynet = nullptr;
	};
}

#define sSceneClientManager slam::Singleton<slam::SceneClientManager>::getInstance()

#endif

/*
 * \file: SceneTaskManager.h
 * \brief: Created by hushouguo at 09:42:08 May 05 2019
 */
 
#ifndef __SCENETASKMANAGER_H__
#define __SCENETASKMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class SceneTaskManager : public Manager<SceneTask> {
		public:
			SceneTaskManager();
			
		public:
			bool init();
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

#define sSceneTaskManager slam::Singleton<slam::SceneTaskManager>::getInstance()

#endif

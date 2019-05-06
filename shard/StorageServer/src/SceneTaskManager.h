/*
 * \file: SceneTaskManager.h
 * \brief: Created by hushouguo at 09:42:08 May 05 2019
 */
 
#ifndef __SCENETASKMANAGER_H__
#define __SCENETASKMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class SceneTaskManager : public Manager<SceneTask> {
		public:	
			bool msgParser(SOCKET socket, CommonMessage* rawmsg);
	};
}

#define sSceneTaskManager slam::Singleton<slam::SceneTaskManager>::getInstance()

#endif

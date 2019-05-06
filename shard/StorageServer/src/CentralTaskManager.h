/*
 * \file: CentralTaskManager.h
 * \brief: Created by hushouguo at 13:43:14 Apr 30 2019
 */
 
#ifndef __CENTRALTASKMANAGER_H__
#define __CENTRALTASKMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class CentralTaskManager : public Manager<CentralTask> {
		public:	
			bool msgParser(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg);
	};
}

#define sCentralTaskManager slam::Singleton<slam::CentralTaskManager>::getInstance()

#endif

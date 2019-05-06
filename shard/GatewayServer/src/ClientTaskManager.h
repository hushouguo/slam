/*
 * \file: ClientTaskManager.h
 * \brief: Created by hushouguo at 13:43:14 Apr 30 2019
 */
 
#ifndef __CLIENTTASKMANAGER_H__
#define __CLIENTTASKMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class ClientTaskManager : public Manager<ClientTask> {
		public:	
			bool msgParser(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg);
			bool sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message);
	};
}

#define sClientTaskManager slam::Singleton<slam::ClientTaskManager>::getInstance()

#endif

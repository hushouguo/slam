/*
 * \file: ClientTaskManager.h
 * \brief: Created by hushouguo at 13:43:14 Apr 30 2019
 */
 
#ifndef __CLIENTTASKMANAGER_H__
#define __CLIENTTASKMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class ClientTaskManager : public Manager<ClientTask> {
		public:
			bool init(const char* address, int port, size_t number = 0);
			void stop();
		
		public:	
			void run();
			bool msgParser(SOCKET socket, CommonMessage* rawmsg);

		private:
			//SOCKET _fds[];
			Easynet* _easynet = nullptr;
	};
}

#define sClientTaskManager slam::Singleton<slam::ClientTaskManager>::getInstance()

#endif

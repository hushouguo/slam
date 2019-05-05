/*
 * \file: CentralTaskManager.h
 * \brief: Created by hushouguo at 13:43:14 Apr 30 2019
 */
 
#ifndef __CENTRALTASKMANAGER_H__
#define __CENTRALTASKMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class CentralTaskManager : public Manager<CentralTask> {
		public:
			bool init(const char* address, int port, size_t number = 0);
			void stop();
		
		public:	
			void run();
			bool msgParser(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg);

		public:
			bool sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message);
			
		private:
			std::vector<Easynet*> _easynets;
			void run(Easynet* easynet);
	};
}

#define sCentralTaskManager slam::Singleton<slam::CentralTaskManager>::getInstance()

#endif

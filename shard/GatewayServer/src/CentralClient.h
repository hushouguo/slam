/*
 * \file: CentralClient.h
 * \brief: Created by hushouguo at 13:43:31 Apr 30 2019
 */
 
#ifndef __CENTRALCLIENT_H__
#define __CENTRALCLIENT_H__

BEGIN_NAMESPACE_SLAM {
	class CentralClient {
		public:
			bool init(const char* address, int port);
			void stop();
		
		public:	
			void run();
			bool msgParser(CommonMessage* rawmsg);
			bool sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			Easynet* _easynet = nullptr;
			SOCKET _socket = EASYNET_ILLEGAL_SOCKET;

		private:
			void registerServer();
	};
}

#define sCentralClient slam::Singleton<slam::CentralClient>::getInstance()

#endif

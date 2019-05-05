/*
 * \file: CentralClient.h
 * \brief: Created by hushouguo at 13:43:31 Apr 30 2019
 */
 
#ifndef __CENTRALCLIENT_H__
#define __CENTRALCLIENT_H__

BEGIN_NAMESPACE_SLAM {
	class CentralClient : public Entry<SOCKET> {
		public:
			CentralClient() : Entry<SOCKET>(EASYNET_ILLEGAL_SOCKET) {
			}
			const char* getClassName() override { return "CentralClient"; }
			
		public:
			bool init(const char* address, int port);
			void stop();
		
		public:	
			void run();
			bool msgParser(CommonMessage* rawmsg);

		public:
			bool sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			Easynet* _easynet = nullptr;
	};
}

#define sCentralClient slam::Singleton<slam::CentralClient>::getInstance()

#endif

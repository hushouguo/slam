/*
 * \file: SceneClient.h
 * \brief: Created by hushouguo at 13:43:42 Apr 30 2019
 */
 
#ifndef __SCENECLIENT_H__
#define __SCENECLIENT_H__

BEGIN_NAMESPACE_SLAM {
	class SceneClient : public Entry<SOCKET> {
		public:
			SceneClient(uint64_t sceneServerid);
			const char* getClassName() override { return "SceneClient"; }
	
		public:
			bool init(const char* address, int port);
			void stop();
		
		public:	
			void run();
			bool msgParser(CommonMessage* rawmsg);

		public:
			bool sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			SOCKET _fd_centralclient = EASYNET_ILLEGAL_SOCKET;
			Easynet* _easynet = nullptr;
	};
}

#endif

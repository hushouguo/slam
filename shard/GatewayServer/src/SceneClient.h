/*
 * \file: SceneClient.h
 * \brief: Created by hushouguo at 13:43:42 Apr 30 2019
 */
 
#ifndef __SCENECLIENT_H__
#define __SCENECLIENT_H__

BEGIN_NAMESPACE_SLAM {
	class SceneClient : public Entry<SOCKET> {
		public:
			SceneClient(Easynet* easynet, SOCKET socket);
			const char* getClassName() override { return "SceneClient"; }
	
		public:
			bool sendMessage(u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			Easynet* _easynet = nullptr;
	};
}

#endif

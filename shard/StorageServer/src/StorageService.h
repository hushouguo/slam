/*
 * \file: StorageService.h
 * \brief: Created by hushouguo at 11:22:55 May 06 2019
 */
 
#ifndef __STORAGESERVICE_H__
#define __STORAGESERVICE_H__

BEGIN_NAMESPACE_SLAM {
	class StorageService {
		public:
			bool init(const char* address, int port);
			void stop();

		public:
			void run();
			bool sendMessage(SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message);

		private:
			Easynet* _easynet = nullptr;
	};
}

#define sStorageService slam::Singleton<slam::StorageService>::getInstance()

#endif

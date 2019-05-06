/*
 * \file: StorageProcessManager.h
 * \brief: Created by hushouguo at 10:21:46 May 06 2019
 */
 
#ifndef __STORAGEPROCESSMANAGER_H__
#define __STORAGEPROCESSMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class StorageProcessManager : public Manager<StorageProcess> {
		public:
			inline int size() { return this->_size; }

		public:
			bool init(int number);
			void stop();
		
		public:
			void serialize(Easynet* easynet, SOCKET socket, StorageSerializeRequest* request);
			void unserialize(Easynet* easynet, SOCKET socket, StorageUnserializeRequest* request);

		private:
			int _size = 0;
	};
}

#define sStorageProcessManager slam::Singleton<slam::StorageProcessManager>::getInstance()

#endif

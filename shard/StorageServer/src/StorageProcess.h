/*
 * \file: StorageProcess.h
 * \brief: Created by hushouguo at 09:49:53 May 05 2019
 */
 
#ifndef __STORAGEPROCESS_H__
#define __STORAGEPROCESS_H__

BEGIN_NAMESPACE_SLAM {
	class MySQL;
	class StorageProcess : public Entry<int> {
		public:
			StorageProcess(int id);
			const char* getClassName() override { return "StorageProcess"; }

		public:
			bool init();
			
		public:
			void serialize(Easynet* easynet, SOCKET socket, StorageSerializeRequest* request);
			void unserialize(Easynet* easynet, SOCKET socket, StorageUnserializeRequest* request);

		private:
			StorageEntityManager _entityManager;
			StorageHandlerManager _handlerManager;
	};
}

#endif

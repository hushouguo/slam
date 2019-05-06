/*
 * \file: StorageProcess.h
 * \brief: Created by hushouguo at 09:49:53 May 05 2019
 */
 
#ifndef __STORAGEPROCESS_H__
#define __STORAGEPROCESS_H__

BEGIN_NAMESPACE_SLAM {
	class StorageProcess : public Runnable {
		public:
			StorageProcess(u32 id);
			const char* getClassName() override { return "StorageProcess"; }

		public:
			void run() override;

		public:
			bool serialize(u32 shard, std::string table, Entity* entity, Message* context);	
	};
}

#endif

/*
 * \file: StorageHandlerManager.h
 * \brief: Created by hushouguo at 10:14:19 May 07 2019
 */
 
#ifndef __STORAGEHANDLERMANAGER_H__
#define __STORAGEHANDLERMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class StorageHandlerManager : public Manager<StorageHandler> {
		public:
			bool init();

		public:
			u64 InsertEntityToTable(u32 shard, std::string table, Entity* entity);
			Entity* RetrieveEntityFromTable(u32 shard, std::string table, u64 entityid);
	};
}

#endif

/*
 * \file: StorageHandler.h
 * \brief: Created by hushouguo at 10:01:46 May 07 2019
 */
 
#ifndef __STORAGEHANDLER_H__
#define __STORAGEHANDLER_H__

BEGIN_NAMESPACE_SLAM {
	class StorageHandler : public Entry<u32> {
		public:
			StorageHandler(u32 shard);
			const char* getClassName() override { return "StorageHandler"; }

		public:
			bool init(std::string host, std::string user, std::string password, std::string database, int port);

		public:
			u64  InsertEntityToTable(u32 shard, std::string table, const Entity* entity);
			bool RetrieveEntityFromTable(u32 shard, std::string table, u64 entityid, Entity* entity);
			bool UpdateEntityToTable(u32 shard, std::string table, u64 entityid, const Entity* entity);
		
		private:
			MySQL* _dbhandler = nullptr;
			MessageStatement* _messageStatement = nullptr;
	};
}

#endif

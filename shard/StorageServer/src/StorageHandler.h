/*
 * \file: StorageHandler.h
 * \brief: Created by hushouguo at 10:01:46 May 07 2019
 */
 
#ifndef __STORAGEHANDLER_H__
#define __STORAGEHANDLER_H__

BEGIN_NAMESPACE_SLAM {
	class StorageHandler : public Entry<int> {
		public:
			StorageHandler(int shard);
			const char* getClassName() override { return "StorageHandler"; }

		public:
			bool init(std::string host, std::string user, std::string password, std::string database, int port);

		public:
			u64  InsertEntityToTable(u32 shard, std::string table, const Entity* entity);
			bool RetrieveEntityFromTable(u32 shard, std::string table, u64 entityid, Entity* entity);
			bool UpdateEntityToTable(u32 shard, std::string table, Entity* entity);
		
		private:
			MySQL* _dbhandler = nullptr;

		private:
			struct FieldDescriptor {
				enum_field_types type;
				u32 flags;
				u64 length;
			};		
			std::unordered_map<std::string, std::unordered_map<std::string, FieldDescriptor>> _tables;
			bool loadField(std::string table);
			void DumpFieldDescriptor();

		private:
			bool UpdateTable(std::string table, const Entity* entity);
			bool CreateTable(std::string table, const Entity* entity);
			bool AlterTable(std::string table, const Entity* entity);

		private:
			const char* GetFieldDescriptor(const Value& value);
			bool NeedExtendField(const FieldDescriptor& descriptor, const Value& value);
	};
}

#endif

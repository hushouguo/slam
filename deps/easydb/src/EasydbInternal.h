/*
 * \file: EasydbInternal.h
 * \brief: Created by hushouguo at 11:24:21 Apr 12 2019
 */
 
#ifndef __EASYDBINTERNAL_H__
#define __EASYDBINTERNAL_H__

namespace db {
	class EasydbInternal : public Easydb {
		public:
			EasydbInternal();
			~EasydbInternal();

		public:
			bool connectServer(EasydbConfig* conf) override;

		public:
			bool createDatabase(std::string) override;
			bool selectDatabase(std::string) override;
			bool deleteDatabase(std::string) override;
			bool findDatabase(std::string) override;
			
		public:
			uint64_t createEntity(std::string table, Entity* entity) override;
			Entity* getEntity(std::string table, uint64_t entityid) override;
			bool deleteEntity(std::string table, uint64_t entityid) override;
			bool runQuery(std::string where, std::vector<Entity*>& entities) override;

		public:
			// MUL KEY
			bool addKey(std::string table, std::string field) override;
			bool removeKey(std::string table, std::string field) override;
			
			// UNI KEY
			bool addUnique(std::string table, std::string field) override;
			bool removeUnique(std::string table, std::string field) override;
			
			// UNSIGNED
			bool addUnsigned(std::string table, std::string field) override;
			bool removeUnsigned(std::string table, std::string field) override;

		private:
			std::string _database;
			MySQL* _dbhandler = nullptr;
		    std::unordered_map<std::string, std::unordered_map<u64, Entity*>> _entities;

		private:
			//bool _isstop = false;
			//std::thread* _threadWorker = nullptr;
			//std::function<int(const void*, size_t)> _spliter;

		private:
		    struct FieldDescriptor {
		        enum_field_types type;
		        u32 flags;
		        u64 length; /* Width of column (create length) */
		    };
		    std::unordered_map<std::string, std::unordered_map<std::string, FieldDescriptor>> _tables;
		    void dumpTables();

			bool extendField(std::string table, Entity* entity);
			//bool insertOrUpdate(std::string table, const Entity* entity);
			bool retrieve(std::string table, uint64_t entityid, Entity* entity);
			bool loadFieldDescriptor(std::string table);
		    bool loadFieldDescriptor();
		    bool createTable(std::string table);
		    u64  insertTable(std::string table, Entity* entity);
		    bool addField(std::string table, const std::string& field_name, enum_field_types field_type);
		    bool alterField(std::string table, const std::string& field_name, enum_field_types field_type);
		    bool deleteEntity(std::string table, uint64_t entityid);
	};
}

#endif

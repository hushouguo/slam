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
			void stop() override;
			bool connectServer(std::string host, std::string user, std::string passwd, int port) override;

		public:
			bool createDatabase(std::string) override;
			bool selectDatabase(std::string) override;
			bool deleteDatabase(std::string) override;
			bool findDatabase(std::string) override;
			
		public:
			bool loadDescriptor(std::string table, std::string filename, std::string name) override;
			bool createObject(std::string table, uint64_t id, const std::string& data) override;
			bool retrieveObject(std::string table, uint64_t id, std::string& data) override;
			bool updateObject(std::string table, uint64_t id, const std::string& data) override;
			bool deleteObject(std::string table, uint64_t id) override;
			bool batchQuery(std::string where, std::unordered_map<uint64_t, std::string>& objects) override;

		private:
			std::string _database;
			MySQL* _dbhandler = nullptr;
		    std::unordered_map<std::string, std::unordered_map<u64, Entity*>> _entities;

		private:
			//bool _isstop = false;
			//std::thread* _threadWorker = nullptr;
			//std::function<int(const void*, size_t)> _spliter;

		private:
		    bool createTable(std::string table);
		    u64  insertTable(std::string table, Entity* entity);
	};
}

#endif

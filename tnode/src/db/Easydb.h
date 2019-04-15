/*
 * \file: Easydb.h
 * \brief: Created by hushouguo at 14:17:56 Apr 15 2019
 */
 
#ifndef __EASYDB_H__
#define __EASYDB_H__

BEGIN_NAMESPACE_TNODE {
	class Easydb {
		public:
			virtual ~Easydb() = 0;
			virtual void stop() = 0;

		public:
			virtual bool connectServer(std::string host, std::string user, std::string passwd, int port) = 0;

		public:
			virtual bool createDatabase(std::string) = 0;
			virtual bool selectDatabase(std::string) = 0;
			virtual bool deleteDatabase(std::string) = 0;
			virtual bool findDatabase(std::string) = 0;

		public:
			virtual bool loadDescriptor(std::string table, std::string filename, std::string name) = 0;
			virtual bool createObject(std::string table, uint64_t id, const std::string& data) = 0;
			virtual bool retrieveObject(std::string table, uint64_t id, std::string& data) = 0;
			virtual bool updateObject(std::string table, uint64_t id, const std::string& data) = 0;
			virtual bool deleteObject(std::string table, uint64_t id) = 0;
			virtual bool batchQuery(std::string where, std::unordered_map<uint64_t, std::string>& objects) = 0;

		public:
			static Easydb* createInstance();
	};
}

#endif

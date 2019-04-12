/*
 * \file: EasydbInternal.cpp
 * \brief: Created by hushouguo at 11:24:24 Apr 12 2019
 */

#include "mysql/MySQLResult.h"
#include "mysql/MySQLStatement.h"
#include "mysql/MySQL.h"
#include "Easydb.h"
#include "EasydbInternal.h"

namespace db {
	EasydbInternal::EasydbInternal() {
	}

	Easydb::~Easydb() {}
	EasydbInternal::~EasydbInternal() {
	}

	int EasydbInternal::connectServer(const char* host, const char* user, const char* passwd, int port) {
		this->_dbhandler = new MySQL();
		bool rc = this->_dbhandler->openDatabase(host, user, passwd, nullptr, port);
		CHECK_RETURN(rc, false, "conf: %s error", conf.c_str());
		
		char database[64];
		snprintf(database, sizeof(database), "shard_%u_%u", this->shard, this->id);
		
		//load databases
		std::set<std::string> results;
		rc = this->dbhandler->loadDatabase(database, results);
		CHECK_RETURN(rc, false, "load database error");
		
		//create database
		if (results.find(database) == results.end()) {
			rc = this->dbhandler->createDatabase(database);
			CHECK_RETURN(rc, false, "create database: %s error", database);
			System << "create database: " << database << " OK";
		}
		
		//select database
		rc = this->dbhandler->selectDatabase(database);
		CHECK_RETURN(rc, false, "select database: %s error", database);
		
		//load table
		results.clear();
		rc = this->dbhandler->loadTable("%", results);
		CHECK_RETURN(rc, false, "load table from database: %s error", database);
		
		//load fields decription
		for (auto& table : results) {
			rc = this->loadField(table);
			CHECK_RETURN(rc, false, "load field: %s error", table.c_str());
		}
		//this->dump();
		
		SafeDelete(this->threadWorker);
		this->threadWorker = new std::thread([this]() {
			this->update();
		});
		
		return true;
	}
	
	int EasydbInternal::connectServer(const char* address, int port) {
		//TODO: fix it
		return -1;
	}
	
	bool EasydbInternal::selectDatabase(std::string) {
	}

	bool EasydbInternal::addEntity(int handle, std::string table, const Entity* entity) {
	}
	
	bool EasydbInternal::modifyEntity(int handle, std::string table, const Entity* entity) {
	}
	
	bool EasydbInternal::loadEntity(int handle, std::string table, uint64_t entityid, Entity* entity) {
	}
	
	bool EasydbInternal::removeEntity(int handle, uint64_t entityid) {
	}
	
	bool EasydbInternal::runQuery(int handle, std::string where, std::vector<Entity*>& entities) {
	}

	// MUL KEY
	bool EasydbInternal::addKey(int handle, std::string table, std::string field) {
	}
	
	bool EasydbInternal::removeKey(int handle, std::string table, std::string field) {
	}
	
	// UNI KEY
	bool EasydbInternal::addUnique(int handle, std::string table, std::string field) {
	}
	
	bool EasydbInternal::removeUnique(int handle, std::string table, std::string field) {
	}
	
	// UNSIGNED
	bool EasydbInternal::addUnsigned(int handle, std::string table, std::string field) {
	}
	
	bool EasydbInternal::removeUnsigned(int handle, std::string table, std::string field) {
	}
}

class EasydbInternal : public Easydb {
	public:
		EasydbInternal();
		~EasydbInternal();

	public:
		int localServer(const char* host, const char* user, const char* passwd, int port) override;
		int connectServer(const char* address, int port) override;

	public:
		bool selectDatabase(std::string) override;

	public:
		bool addEntity(int handle, std::string table, const Entity* entity) override;
		bool modifyEntity(int handle, std::string table, const Entity* entity) override;
		bool loadEntity(int handle, std::string table, uint64_t entityid, Entity* entity) override;
		bool removeEntity(int handle, uint64_t entityid) override;
		bool runQuery(int handle, std::string where, std::vector<Entity*>& entities) override;

	public:
		// MUL KEY
		bool addKey(int handle, std::string table, std::string field) override;
		bool removeKey(int handle, std::string table, std::string field) override;
		// UNI KEY
		bool addUnique(int handle, std::string table, std::string field) override;
		bool removeUnique(int handle, std::string table, std::string field) override;
		// UNSIGNED
		bool addUnsigned(int handle, std::string table, std::string field) override;
		bool removeUnsigned(int handle, std::string table, std::string field) override;

	private:
		MySQL* _dbhandler = nullptr;
};


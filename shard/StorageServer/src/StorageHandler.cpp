/*
 * \file: StorageHandler.cpp
 * \brief: Created by hushouguo at 10:14:00 May 07 2019
 */

#include "common.h"
#include "mysql/MySQLResult.h"
#include "mysql/MySQLStatement.h"
#include "mysql/MySQL.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"
#include "StorageEntity.h"
#include "StorageEntityManager.h"
#include "MessageStatement.h"
#include "StorageHandler.h"
#include "StorageHandlerManager.h"

#define MYSQL_VARCHAR_ASCII_MAXSIZE		65532
#define MYSQL_VARCHAR_UTF8_MAXSIZE		21845
#define MYSQL_VARCHAR_GBK_MAXSIZE		32766

BEGIN_NAMESPACE_SLAM {
	StorageHandler::StorageHandler(u32 shard) : Entry<u32>(shard) {
	}

	u64 StorageHandler::InsertEntityToTable(u32 shard, std::string table, const Entity* entity) {
		assert(shard == this->id);
		CHECK_RETURN(this->_dbhandler && this->_messageStatement, 0, "StorageHandler: %d not initiated", shard);
		u64 entityid = 0;
		try {
			bool rc = this->_messageStatement->CreateMessage(table, entity, &entityid);
			CHECK_RETURN(rc, 0, "CreateMessage: %s, shard: %d error", table.c_str(), shard);
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, 0, "CreateMessage exception:%s, shard:%d, table:%s", e.what(), shard, table.c_str());
		}		
		return entityid;
	}
	
	bool StorageHandler::RetrieveEntityFromTable(u32 shard, std::string table, u64 entityid, Entity* entity) {
		assert(shard == this->id);
		CHECK_RETURN(this->_dbhandler && this->_messageStatement, 0, "StorageHandler: %d not initiated", shard);
		try {
			return this->_messageStatement->RetrieveMessage(table, entityid, entity);
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "RetrieveMessage exception:%s, shard:%d, table:%s, entityid:%lld", e.what(), shard, table.c_str(), entityid);
		}
		return false;		
	}

	bool StorageHandler::UpdateEntityToTable(u32 shard, std::string table, u64 entityid, const Entity* entity) {
		assert(shard == this->id);
		CHECK_RETURN(this->_dbhandler && this->_messageStatement, 0, "StorageHandler: %d not initiated", shard);
		try {
			return this->_messageStatement->UpdateMessage(table, entityid, entity);
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "UpdateMessage exception:%s, shard:%d, table:%s, entityid:%lld", e.what(), shard, table.c_str(), entityid);
		}
		return false;		
	}
	
	bool StorageHandler::init(std::string host, std::string user, std::string password, std::string database, int port) {
		SafeDelete(this->_dbhandler);
		SafeDelete(this->_messageStatement);
		this->_dbhandler = new MySQL();
		bool rc = this->_dbhandler->openDatabase(
			host.c_str(),
			user.c_str(),
			password.c_str(),
			nullptr,
			port
		);
		CHECK_RETURN(rc, false, "connectDatabase(%s:%s:%s:%d) error",
			host.c_str(),
			user.c_str(),
			password.c_str(),
			port
		);

		//
		// create if not exists
		rc = this->_dbhandler->createDatabase(database);
		CHECK_RETURN(rc, false, "create database: %s error", database.c_str());
		
		rc = this->_dbhandler->selectDatabase(database);
		CHECK_RETURN(rc, false, "select database: %s error", database.c_str());

		this->_messageStatement = new MessageStatement(this->_dbhandler);
		
		return true;
	}

	StorageHandler::~StorageHandler() {
		SafeDelete(this->_messageStatement);
		SafeDelete(this->_dbhandler);		
	}
}


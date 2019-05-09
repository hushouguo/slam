/*
 * \file: StorageHandler.cpp
 * \brief: Created by hushouguo at 10:14:00 May 07 2019
 */

#include "common.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"
#include "StorageEntity.h"
#include "StorageEntityManager.h"
#include "StorageHandler.h"

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
		bool rc = this->_messageStatement->CreateMessage(table, entity, &entityid);
		CHECK_RETURN(rc, 0, "CreateMessage: %s, shard: %d error", table.c_str(), shard);
		return entityid;
	}
	
	bool StorageHandler::RetrieveEntityFromTable(u32 shard, std::string table, u64 entityid, Entity* entity) {
#if false	
		assert(shard == this->id);
		assert(this->_dbhandler);
		std::ostringstream sql;
		sql << "SELECT * FROM `" << table << "` WHERE id = " << entityid;
		MySQLResult* result = this->_dbhandler->runQuery(sql.str());
		if (!result) {
			return false;
		}
		entity->Clear();
		
		u32 rowNumber = result->rowNumber();
		if (rowNumber == 0) {
			SafeDelete(result);
			return false;
		}
		
		MYSQL_FIELD* fields = result->fetchField(); 
		MYSQL_ROW rows = result->fetchRow();
		assert(rows);
		
		u32 fieldNumber = result->fieldNumber();
		for (u32 i = 0; i < fieldNumber; ++i) {
			const MYSQL_FIELD& field = fields[i];
			switch (field.type) {
				// NULL
				case MYSQL_TYPE_NULL:
					entity->mutable_values()->operator[](field.org_name).set_type(valuetype_nil);
					break; 
				
				// bool
				case MYSQL_TYPE_TINY:
					entity->mutable_values()->operator[](field.org_name).set_type(valuetype_bool);
					entity->mutable_values()->operator[](field.org_name).set_value_bool(std::stoi(rows[i]) != 0);
					break; 
			
				// int64
				case MYSQL_TYPE_SHORT:
				case MYSQL_TYPE_LONG:
					entity->mutable_values()->operator[](field.org_name).set_type(valuetype_int64);
					entity->mutable_values()->operator[](field.org_name).set_value_int64(std::stol(rows[i]));
					break; 
					
				case MYSQL_TYPE_LONGLONG:
					if (strcmp(field.org_name, "id") == 0) {
						entity->set_id(std::stoull(rows[i]));
					}
					else {
						entity->mutable_values()->operator[](field.org_name).set_type(valuetype_int64);
						entity->mutable_values()->operator[](field.org_name).set_value_int64(std::stoll(rows[i]));
					}				
					break; 
			
				// string
				case MYSQL_TYPE_TIMESTAMP:
				case MYSQL_TYPE_STRING:
				case MYSQL_TYPE_VAR_STRING:
				case MYSQL_TYPE_TINY_BLOB:
				case MYSQL_TYPE_BLOB:
				case MYSQL_TYPE_MEDIUM_BLOB:
				case MYSQL_TYPE_LONG_BLOB:
					entity->mutable_values()->operator[](field.org_name).set_type(valuetype_string);
					entity->mutable_values()->operator[](field.org_name).set_value_string(rows[i]);
					break; 
			
				// float
				case MYSQL_TYPE_DOUBLE:
				case MYSQL_TYPE_FLOAT:
					entity->mutable_values()->operator[](field.org_name).set_type(valuetype_float);
					entity->mutable_values()->operator[](field.org_name).set_value_float(std::stof(rows[i]));
					break; 
			
				default: 
					SafeDelete(result);
					CHECK_RETURN(false, false, "unknown MYSQL type: %d, field: %s, table: %s, shard: %d", field.type, field.org_name, table.c_str(), shard);
			}
		}
		
		SafeDelete(result);
#endif		
		return true;
	}

	bool StorageHandler::UpdateEntityToTable(u32 shard, std::string table, Entity* entity) {
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
}


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
		assert(this->_dbhandler);
		CHECK_RETURN(this->UpdateTable(table, entity), 0, "table: %s, shard: %d update error", table.c_str(), shard);
		std::ostringstream sql, sql_fields, sql_values;
		for (auto& i : entity->values()) {
			if (sql_fields.rdbuf()->in_avail()) {
				sql_fields << ",";
				sql_values << ",";
			}
			sql_fields << "`" << i.first << "`";
			auto& value = i.second;
			switch (value.type()) {
				case valuetype_nil: sql_values << "NULL"; break;
				case valuetype_int64: sql_values << value.value_int64(); break;
				case valuetype_string: sql_values << "'" << value.value_string() << "'"; break;
				case valuetype_float: sql_values << value.value_float; break;
				case valuetype_bool: sql_values << (value.value_bool() ? 1 : 0); break;
				default: 
				CHECK_RETURN(false, 0, "illegal value type: %d, name: %s, table: %s, shard: %d", value.type(), i.first.c_str(), table.c_str(), shard);
			}
		}
		if (entity->id() != 0) {
			sql_fields << ",id";
			sql_values << "," << entity->id();
		}
		sql << "INSERT INTO `" << table << "` (" << sql_fields.str() << ") VALUES (" << sql_values.str() << ")";
		bool rc = this->_dbhandler->runCommand(sql.str());
		CHECK_RETURN(rc, 0, "run sql: %s error", sql.str().c_str());
		//todo: cache entity
		return entity->id() == 0 ? this->_dbhandler->insertId() : entity->id();
	}
	
	bool StorageHandler::RetrieveEntityFromTable(u32 shard, std::string table, u64 entityid, Entity* entity) {
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
		return true;
	}

	bool StorageHandler::UpdateEntityToTable(u32 shard, std::string table, Entity* entity) {
		return false;
	}

	bool StorageHandler::UpdateTable(std::string table, const Entity* entity) {
		if (this->_tables.find(table) == this->_tables.end()) {
			bool rc = this->CreateTable(table, entity);
			if (rc) {
				this->_tables.insert(table);
			}
			return rc;
		}
		return this->AlterTable(table, entity);
	}

	bool StorageHandler::CreateTable(std::string table, const Entity* entity) {		
		assert(this->_dbhandler);
		assert(entity);
		std::ostringstream sql;
		sql << "CREATE TABLE IF NOT EXISTS `" << table << "`(";
		sql << "`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY";
		for (auto& i : entity->values()) {
			sql << ", `" << i.first << "` ";
			const char* fieldString = this->GetFieldDescriptor(i.second);
			CHECK_RETURN(fieldString, false, "table: %s, field: %s, valuetype: %d", table.c_str(), i.first.c_str(), i.second.type());
			sql << fieldString;
		}
		sql << ") ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8";
		bool rc = this->_dbhandler->runCommand(sql.str());
		CHECK_RETURN(rc, 0, "run sql: %s error", sql.str().c_str());
		return rc;
	}
	
	bool StorageHandler::AlterTable(std::string table, const Entity* entity) {
#if 0	
		assert(this->_dbhandler);
		assert(entity);
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];		
		std::ostringstream sql;
		sql << "ALTER TABLE `" << table << "` MODIFY `" << field_name << "` " << m2string[field_type] << " NOT NULL";
		sql << "ALTER TABLE `" << table << "` ADD `" << field_name << "` " << m2string[field_type] << " NOT NULL";
		for (auto& i : entity->values()) {
			auto& value = i.second;
			if (desc_fields.find(i.first) == desc_fields.end()) {
				sql << " ADD ";
			}
			else {
				if (value.type() == )
				sql << " MODIFY ";
			}			
			sql << "`" << i.first << "` ";
			
			switch (value.type()) {
				case valuetype_nil: 
					CHECK_RETURN(false, false, "table: %s, found NULL field: %s", table.c_str(), i.first.c_str());
				case valuetype_int64:
					if (value.value_int64() > INT32_MAX || value.value_int64() < INT32_MIN) {
						sql << "BIGINT NOT NULL";
					}
					else {
						sql << "INT NOT NULL";
					}
					break;
				case valuetype_string:
					if (value.value_string().length() < MYSQL_VARCHAR_UTF8_MAXSIZE) {
						if (value.value_string().length() < 32) {
							sql << "VARCHAR(32) NOT NULL";
						}
						else if (value.value_string().length() < 128) {
							sql << "VARCHAR(128) NOT NULL";
						}
						else if (value.value_string().length() < 256) {
							sql << "VARCHAR(256) NOT NULL";
						}
						else if (value.value_string().length() < 1024) {
							sql << "VARCHAR(1024) NOT NULL";
						}
						else {
							sql << "VARCHAR(" << (MYSQL_VARCHAR_UTF8_MAXSIZE - 1) << ") NOT NULL";
						}
					}
					else {
						sql << "LONGTEXT NOT NULL";
					}
					break;
				case valuetype_float:
					sql << "FLOAT NOT NULL";
					break;
				case valuetype_bool:
					sql << "TINYINT NOT NULL";
					break;
				default: 
				CHECK_RETURN(false, 0, "illegal value type: %d, name: %s, table: %s, shard: %d", value.type(), i.first.c_str(), table.c_str(), shard);
			}
		}
		sql << ") ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8";
		bool rc = this->_dbhandler->runCommand(sql.str());
		CHECK_RETURN(rc, 0, "run sql: %s error", sql.str().c_str());
		return rc;
#endif
		return true;
	}

	const char* StorageHandler::GetFieldDescriptor(const Value& value) {
		switch (value.type()) {
			case valuetype_int64: 
				return (value.value_int64() > INT32_MAX || value.value_int64() < INT32_MIN) ? "BIGINT NOT NULL" : "INT NOT NULL";
			case valuetype_string:
				if (value.value_string().length() < MYSQL_VARCHAR_UTF8_MAXSIZE) {
					if (value.value_string().length() < 32) {
						return "VARCHAR(32) NOT NULL";
					}
					else if (value.value_string().length() < 128) {
						return "VARCHAR(128) NOT NULL";
					}
					else if (value.value_string().length() < 256) {
						return "VARCHAR(256) NOT NULL";
					}
					else if (value.value_string().length() < 1024) {
						return "VARCHAR(1024) NOT NULL";
					}
					else {
						return "VARCHAR(21844) NOT NULL";
					}
				}
				return "LONGTEXT NOT NULL";
			case valuetype_float: return "FLOAT NOT NULL";
			case valuetype_bool: return "TINYINT NOT NULL";
			case valuetype_nil:
			default: break;
		}
		return nullptr;
	}

	bool StorageHandler::NeedExtendField(const FieldDescriptor& descriptor, const Value& value) {
#if 0	
		switch (value.type()) {
			case valuetype_int64: return descriptor.length < value.value_int64();
			
				return descriptor.type == MYSQL_TYPE_SHORT
				return (value.value_int64() > INT32_MAX || value.value_int64() < INT32_MIN) ? "BIGINT NOT NULL" : "INT NOT NULL";
			case valuetype_string:
				if (value.value_string().length() < MYSQL_VARCHAR_UTF8_MAXSIZE) {
					if (value.value_string().length() < 32) {
						return "VARCHAR(32) NOT NULL";
					}
					else if (value.value_string().length() < 128) {
						return "VARCHAR(128) NOT NULL";
					}
					else if (value.value_string().length() < 256) {
						return "VARCHAR(256) NOT NULL";
					}
					else if (value.value_string().length() < 1024) {
						return "VARCHAR(1024) NOT NULL";
					}
					else {
						return "VARCHAR(21844) NOT NULL";
					}
				}
				return "LONGTEXT NOT NULL";
			case valuetype_float: return "FLOAT NOT NULL";
			case valuetype_bool: return "TINYINT NOT NULL";
			case valuetype_nil:
			default: break;
		}
		return nullptr;
#endif
		return true;
	}

	// 
	// field descriptor
	bool StorageHandler::loadField(std::string table) {
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];
	
		std::ostringstream sql;
		sql << "SELECT * FROM `" << table.c_str() << "` LIMIT 1";
		
		MySQLResult* result = this->dbhandler->runQuery(sql.str());
		assert(result);
		u32 fieldNumber = result->fieldNumber();
		MYSQL_FIELD* fields = result->fetchField();
		for (u32 i = 0; i < fieldNumber; ++i) {
			const MYSQL_FIELD& field = fields[i];
			//CHECK_RETURN(ContainsKey(m2string, field.type), false, "table: %s found not support field: %s, type: %d", table.c_str(), field.org_name, field.type);
			FieldDescriptor& fieldDescriptor = desc_fields[field.org_name];
			fieldDescriptor.type = field.type;
			fieldDescriptor.flags = field.flags;
			fieldDescriptor.length = field.length;
		}
		SafeDelete(result);
		
		return true;
	}

	void StorageHandler::DumpFieldDescriptor() {
		Debug << "shard: " << this->id;
		for (auto& i : this->_tables) {
			Debug << "\ttable: " << i.first;
			for (auto& v : i.second) {
				Debug << "\t\tfield: " << v.first << ", length: " << v.second.length;
			}
		}
	}
	
	bool StorageHandler::init(std::string host, std::string user, std::string password, std::string database, int port) {
		assert(this->_dbhandler == nullptr);
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

		//
		// load table
		std::set<std::string> tables;
		tables.clear();
		rc = this->_dbhandler->loadTable("%", tables);
		CHECK_RETURN(rc, false, "load table from database: %s error", database.c_str());

		//
		// load fields decription
		for (auto& table : tables) {
			rc = this->loadField(table);
			CHECK_RETURN(rc, false, "load field: %s error", table.c_str());
		}

		this->DumpFieldDescriptor();

		return true;
	}
}


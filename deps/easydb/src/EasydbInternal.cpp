/*
 * \file: EasydbInternal.cpp
 * \brief: Created by hushouguo at 11:24:24 Apr 12 2019
 */

#include "Database.h"

#ifndef IS_UNI_KEY
#define IS_UNI_KEY(n)	((n) & UNIQUE_KEY_FLAG)
#endif

#ifndef IS_MUL_KEY
#define IS_MUL_KEY(n)	((n) & MULTIPLE_KEY_FLAG)
#endif

#ifndef IS_UNSIGNED
#define IS_UNSIGNED(n)	((n) & UNSIGNED_FLAG)
#endif

#define DEF_VARCHAR_LENGTH	128

namespace db {
	EasydbInternal::EasydbInternal() {
	}

	Easydb::~Easydb() {}
	EasydbInternal::~EasydbInternal() {
		if (this->_dbhandler) {
			//TODO: flush dirty entity to db
			this->_dbhandler->closeDatabase();		
		}
	}

	bool EasydbInternal::connectServer(const char* host, const char* user, const char* passwd, int port) {
		this->_dbhandler = new MySQL();
		bool rc = this->_dbhandler->openDatabase(host, user, passwd, nullptr, port);
		CHECK_RETURN(rc, false, "connectServer(%s:%s:%s:%d) error", host, user, passwd, port);
		return true;
	}

	bool EasydbInternal::connectServer(const char* address, int port) {
		Error("connectServer not implement!");
		//TODO: fix it
		return false;
	}
	
	bool EasydbInternal::createDatabase(std::string database) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		bool rc = this->_dbhandler->createDatabase(database);
		CHECK_RETURN(rc, false, "create database: %s error", database);
		return true;
	}
	
	bool EasydbInternal::selectDatabase(std::string database) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		bool rc = this->_dbhandler->selectDatabase(database);
		CHECK_RETURN(rc, false, "select database: %s error", database);
		rc = this->loadFieldDescriptor(); // load all of tables for field descriptor
		if (!rc) {
			this->_dbhandler->closeDatabase();
			CHECK_RETURN(rc, false, "load all of tables error");
		}
		this->_database = database;
		return true;
	}

	bool EasydbInternal::serialize(std::string table, const Entity* entity) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		CHECK_RETURN(this->_entities.find(table) != this->_entities.end(), false, "table: %s not exist", table.c_str());
		bool rc = this->insertOrUpdate(table, entity);
		CHECK_RETURN(rc, false, "InsertOrUpdate entity: 0x%lx error", entity->id);
		std::unordered_map<u64, Entity*>& entities = this->_entities[table];
		auto i = entities.find(entity->id);
		if (i == entities.end()) {
			// this is a new entity
			entities.insert(std::make_pair(entity->id, entity));
		}
		return true;
	}
		
	Entity* EasydbInternal::unserialize(std::string table, uint64_t entityid) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		CHECK_RETURN(this->_entities.find(table) != this->_entities.end(), false, "table: %s not exist", table.c_str());
		std::unordered_map<u64, Entity*>& entities = this->_entities[table];
		auto i = entities.find(entityid);
		if (i != entities.end()) {
			return i->second;
		}
		Entity* entity = new Entity(entityid);
		bool rc = this->retrieve(table, entityid, entity);
		if (!rc) {
			SafeDelete(entity);
			CHECK_RETURN(false, nullptr, "retrieve entity: 0x%lx error", entityid);
		}
		entities.insert(std::make_pair(entityid, entity));
		return entity;
	}
	
	bool EasydbInternal::removeEntity(std::string table, uint64_t entityid) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		CHECK_RETURN(this->_entities.find(table) != this->_entities.end(), false, "table: %s not exist", table.c_str());
		std::unordered_map<u64, Entity*>& entities = this->_entities[table];
		auto i = entities.find(entityid);
		if (i != entities.end()) {
			Entity* entity = i->second;
			SafeDelete(entity);
			entities.erase(i);
		}
		return this->deleteEntity(table, entityid);
	}
	
	bool EasydbInternal::runQuery(std::string where, std::vector<Entity*>& entities) {
		return false;
	}

	// MUL KEY
	bool EasydbInternal::addKey(std::string table, std::string field) {
		return false;
	}
	
	bool EasydbInternal::removeKey(std::string table, std::string field) {
		return false;
	}
	
	// UNI KEY
	bool EasydbInternal::addUnique(std::string table, std::string field) {
		return false;
	}
	
	bool EasydbInternal::removeUnique(std::string table, std::string field) {
		return false;
	}
	
	// UNSIGNED
	bool EasydbInternal::addUnsigned(std::string table, std::string field) {
		return false;
	}
	
	bool EasydbInternal::removeUnsigned(std::string table, std::string field) {
		return false;
	}


	//
	// insert or update entity to db
	bool EasydbInternal::insertOrUpdate(std::string table, const Entity* entity) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		if (!ContainsKey(this->_tables, table)) {
			bool rc = this->createTable(table, entity);
			CHECK_RETURN(rc, false, "create table: %s error", table);
			rc = this->loadFieldDescriptor(table);
			CHECK_RETURN(rc, false, "load field from table: %s error", table);
		}
		
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];
		
		std::ostringstream sql_fields, sql_insert, sql_update;
		for (auto& iterator : entity->values) {
			const Entity::Value& value = iterator.second;
			CHECK_RETURN(valid_type(value.type), false, "illegal ValueType: %d", value.type);		
			enum_field_types field_type = convert(value);
		
			// add new field
			if (!ContainsKey(desc_fields, iterator.first)) {
				bool rc = this->addField(table, iterator.first, field_type);
				CHECK_RETURN(rc, false, "add new field: %s, type: %d error", iterator.first.c_str(), value.type);
				this->loadFieldDescriptor(table);//NOTE: dont reload field descriptor
			}
		
			const FieldDescriptor& fieldDescriptor = desc_fields[iterator.first];
			CHECK_RETURN(fieldDescriptor.type == field_type, false, "fieldDescriptor.type: %d, field_type: %d", fieldDescriptor.type, field_type);
					
			if (sql_fields.tellp() > 0) { sql_fields << ","; }
			if (sql_insert.tellp() > 0) { sql_insert << ","; }
			if (sql_update.tellp() > 0) { sql_update << ","; }
		
			sql_fields << "`" << iterator.first << "`";
		
			switch (value.type) {
				case Entity::type_integer: 
					sql_insert << value.value_integer;
					sql_update << "`" << iterator.first << "`=" << value.value_integer;
					break;
		
				case Entity::type_float:
					sql_insert << value.value_float;
					sql_update << "`" << iterator.first << "`=" << value.value_float;
					break;
		
				case Entity::type_bool:
					sql_insert << value.value_bool;
					sql_update << "`" << iterator.first << "`=" << value.value_bool;
					break;
		
				case Entity::type_string:
					sql_insert << "'" << value.value_string << "'";
					sql_update << "`" << iterator.first << "`=" << "'" << value.value_string << "'";
					break;
		
				default: CHECK_RETURN(false, false, "illegal value type: %d", value.type); break;
			}
		}
		
		std::ostringstream sql;
		sql << "INSERT INTO `" << table << "` (" << sql_fields.str() << ") VALUES (" << sql_insert.str();
		sql << ") ON DUPLICATE KEY UPDATE " << sql_update.str();
		//Trace << "serialize sql: " << sql.str();
		return this->_dbhandler->runCommand(sql.str());	
	}

	//
	// load entity from db
	bool EasydbInternal::retrieve(std::string table, uint64_t entityid, Entity* entity) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];
		
		std::ostringstream sql;
		sql << "SELECT * FROM `" << table << "` WHERE id = " << entityid;
		
		MySQLResult* result = this->_dbhandler->runQuery(sql.str());
		CHECK_RETURN(result, false, "Query: %s error", sql.str().c_str());
		
		u32 rowNumber = result->rowNumber();
		if (rowNumber == 0) {
			SafeDelete(result);
			return false;
		}
		
		u32 fieldNumber = result->fieldNumber();
		if (fieldNumber != desc_fields.size()) {
			SafeDelete(result);
			CHECK_RETURN(fieldNumber == desc_fields.size(), false, "fieldNumber: %u, fields: %ld", fieldNumber, desc_fields.size());
		}
		
		MYSQL_FIELD* fields = result->fetchField(); 
		MYSQL_ROW row = result->fetchRow();
		assert(row);
				
		for (u32 i = 0; i < fieldNumber; ++i) {
			const MYSQL_FIELD& field = fields[i];
			CHECK_RETURN(desc_fields.find(field.org_name) != desc_fields.end(), false, "org_name: %s not exist, table: %s", field.org_name, table);
			CHECK_RETURN(valid_type(field.type), false, "illegal field.type: %d", field.type);
			bool is_unsigned = IS_UNSIGNED(field.flags);
			try {
				fieldvalue(field.type, entity->GetValue(field.org_name), row[i], is_unsigned);
			} 
			catch(std::exception& e) {
				Error("field: %s, type: %d, convert except: %s", field.org_name, field.type, e.what());
			}
		}
		SafeDelete(result);
		
		return true;
	}

	//
	// load field descriptor for all of tables
	bool EasydbInternal::loadFieldDescriptor() {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");

		// load all of tables
		std::set<std::string> tables;
		bool rc = this->_dbhandler->loadTable("%", tables);
		CHECK_RETURN(rc, false, "load all of tables from database: %s error", this->_database.c_str());

		this->_tables.clear();
		this->_entities.clear(); // clean cache of entity
		
		//load fields decription
		for (auto& table : tables) {
			rc = this->loadFieldDescriptor(table);
			CHECK_RETURN(rc, false, "load field: %s error", table.c_str());
		}

		return true;
	}

	//
	// load special field descriptor of table
	bool EasydbInternal::loadFieldDescriptor(std::string table) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];
		desc_fields.clear();
		
		std::ostringstream sql;
		sql << "SELECT * FROM `" << table.c_str() << "` LIMIT 1";
		
		MySQLResult* result = this->_dbhandler->runQuery(sql.str());
		assert(result);
		u32 fieldNumber = result->fieldNumber();
		MYSQL_FIELD* fields = result->fetchField();
		for (u32 i = 0; i < fieldNumber; ++i) {
			const MYSQL_FIELD& field = fields[i];
			CHECK_RETURN(valid_type(field.type), false, "table: %s found not support field: %s, type: %d", table.c_str(), field.org_name, field.type);
			FieldDescriptor& fieldDescriptor = desc_fields[field.org_name];
			fieldDescriptor.type = field.type;
			fieldDescriptor.flags = field.flags;
			fieldDescriptor.length = field.length;
		}
		
		SafeDelete(result);		
		return true;
	}

	//
	// create new table
	bool EasydbInternal::createTable(std::string table) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		std::ostringstream sql;
		sql << "CREATE TABLE `" << table << "` (`id` BIGINT UNSIGNED NOT NULL PRIMARY KEY ";
		sql << ") ENGINE=InnoDB DEFAULT CHARSET=utf8";
		return this->_dbhandler->runCommand(sql.str());
	}

	//
	// add new field
	bool EasydbInternal::addField(std::string table, const std::string& field_name, enum_field_types field_type) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		std::ostringstream sql;
		sql << "ALTER TABLE `" << table << "` ADD `" << field_name << "` " << fieldstring(field_type) << " NOT NULL";
		return this->_dbhandler->runCommand(sql.str());
	}

	//
	// delete entity
	bool EasydbInternal::deleteEntity(std::string table, uint64_t entityid) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		std::ostringstream sql;
		sql << "DELETE FROM `" << table << "` WHERE `id` = " << entityid;
		return this->_dbhandler->runCommand(sql.str());
	}
}


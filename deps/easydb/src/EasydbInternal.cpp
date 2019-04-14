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

	bool EasydbInternal::connectServer(EasydbConfig* conf) {
		CHECK_RETURN(conf, false, "invalid `conf`");
		this->_dbhandler = new MySQL();
		bool rc = this->_dbhandler->openDatabase(
			conf->host.c_str(), 
			conf->user.c_str(), 
			conf->passwd.c_str(), 
			conf->database.length() > 0 ? conf->database.c_str() : nullptr, 
			conf->port);
		CHECK_RETURN(rc, false, "connectServer(%s:%s:%s:%d) error", 
			conf->host.c_str(), 
			conf->user.c_str(), 
			conf->passwd.c_str(), 
			conf->port);
		return true;
	}
	
	bool EasydbInternal::createDatabase(std::string database) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		bool rc = this->_dbhandler->createDatabase(database);
		CHECK_RETURN(rc, false, "create database: %s error", database.c_str());
		return true;
	}

	//
	// selectDatabase & reload all of tables
	bool EasydbInternal::selectDatabase(std::string database) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		bool rc = this->_dbhandler->selectDatabase(database);
		CHECK_RETURN(rc, false, "select database: %s error", database.c_str());
		rc = this->loadFieldDescriptor(); // load all of tables for field descriptor
		if (!rc) {
			this->_dbhandler->closeDatabase();
			CHECK_RETURN(rc, false, "load all of tables error");
		}
		this->_database = database;
		return true;
	}

	bool EasydbInternal::deleteDatabase(std::string database) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		return this->_dbhandler->deleteDatabase(database);
	}

	bool EasydbInternal::findDatabase(std::string database) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		return this->_dbhandler->findDatabase(database);
	}


	uint64_t EasydbInternal::createEntity(std::string table, Entity* entity) {
		CHECK_RETURN(this->_dbhandler, 0, "not connectServer");
		//
		// create table if not exists
		if (this->_tables.find(table) == this->_tables.end()) { 
			bool rc = this->createTable(table);
			CHECK_RETURN(rc, 0, "create table: %s error", table.c_str());
			rc = this->loadFieldDescriptor(table);
			CHECK_RETURN(rc, 0, "load field from table: %s error", table.c_str());
		}

		std::unordered_map<u64, Entity*>& entities = this->_entities[table];
		
		//
		// insert entity to table
		u64 entityid = this->insertTable(table, entity);
		CHECK_RETURN(entityid != 0, 0, "createEntity: %s error", table.c_str());
		CHECK_RETURN(entities.find(entityid) == entities.end(), 0, "duplicate entityid: 0x%lx", entityid);

		if (entity->ID() == 0) {
			//
			// reset entity->id
			entity->ID(entityid);
		}
		else {
			assert(entity->ID() == entityid);
		}

		//
		// reset dirty flags
		entity->clearDirty();
		
		//
		// add new entity to cache
		entities.insert(std::make_pair(entity->id, entity));

		return entityid;
	}

	Entity* EasydbInternal::getEntity(std::string table, uint64_t entityid) {
		CHECK_RETURN(this->_dbhandler, nullptr, "not connectServer");
		//
		// check table if exists
		CHECK_RETURN(this->_tables.find(table) != this->_tables.end(), nullptr, "table: %s not exist", table.c_str());
		
		//
		// check cache
		std::unordered_map<u64, Entity*>& entities = this->_entities[table];
		auto i = entities.find(entityid);
		if (i != entities.end()) {	return i->second; }

		//
		// create new entity
		Entity* entity = new Entity();
		bool rc = this->retrieve(table, entityid, entity);
		if (!rc) {
			SafeDelete(entity);
			CHECK_RETURN(false, nullptr, "retrieve entity: 0x%lx error", entityid);
		}

		//
		// reset dirty flags
		entity->clearDirty();

		//
		// cache entity
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
	// extend field of table by entity
	bool EasydbInternal::extendField(std::string table, Entity* entity) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		//
		// check table
		CHECK_RETURN(this->_tables.find(table) != this->_tables.end(), false, "table: %s not exist", table.c_str());
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];

		//
		// iterator all of values
		for (auto& iterator : entity->values()) {
			const Entity::Value& value = iterator.second;
			CHECK_RETURN(valid_type(value.type), false, "extendField found illegal ValueType: %d", value.type); 	
			enum_field_types field_type = convert(value);
			// add new field
			if (desc_fields.find(iterator.first) == desc_fields.end()) {
				bool rc = this->addField(table, iterator.first, field_type);
				CHECK_RETURN(rc, false, "add new field: %s, type: %d error", iterator.first.c_str(), value.type);
				this->loadFieldDescriptor(table);//NOTE: dont reload field descriptor
			}
			// extend old field
			else {
				const FieldDescriptor& fieldDescriptor = desc_fields[iterator.first];
				if (field_type != fieldDescriptor.type) {
					bool rc = this->alterField(table, iterator.first, field_type);
					CHECK_RETURN(rc, false, "alter table: %s error", table.c_str());
				}
			}
		}

		return true;
	}

	//
	// insert entity to table
	u64 EasydbInternal::insertTable(std::string table, Entity* entity) {
		CHECK_RETURN(this->_dbhandler, 0, "not connectServer");
		//
		// check table
		CHECK_RETURN(this->_tables.find(table) != this->_tables.end(), 0, "table: %s not exist", table.c_str());

		//
		// check entity if empty
		size_t n = 0, size = entity->ValueSize();
		CHECK_RETURN(size > 0, 0, "entity is a empty object");

		//
		// extend fields of table
		bool rc = this->extendField(table, entity);
		CHECK_RETURN(rc, 0, "extend field table: %s error", table.c_str());
		
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];
		const std::unordered_map<std::string, Entity::Value>& values = entity->values();

		std::ostringstream sql_fields, sql_values;
		
		//
		// sql statement
		MySQLStatement stmt(this->_dbhandler);
		unsigned long lengths[size];
		MYSQL_BIND params[size];
		memset(params, 0, sizeof(params));
		for (auto& iterator : values) {
			assert(n < size);
			CHECK_RETURN(desc_fields.find(iterator.first) != desc_fields.end(), 0, "table: %s not exist field: %s", table.c_str(), iterator.first.c_str());
			const FieldDescriptor& fieldDescriptor = desc_fields[iterator.first];
			const Entity::Value& value = iterator.second;

			if (sql_fields.tellp() > 0) { sql_fields << ","; }
			sql_fields << "`" << iterator.first << "`";

			if (sql_values.tellp() > 0) { sql_values << ","; }
			sql_values << "?";
			
			lengths[n] = value.Size();
			params[n].buffer_type = fieldDescriptor.type;
			params[n].buffer = value.Buffer();
			params[n].is_unsigned = false;
			params[n].length = &lengths[n];
			++n;
		}

		//
		// insert ID
		if (entity->ID() != 0) {
			sql_fields << ", ID";
			sql_values << ", " << entity->ID();
		}
		
		std::ostringstream o;
		o << "INSERT INTO `" << table << "` (" << sql_fields.str() << ") VALUES (" << sql_values.str() << ")";
		const std::string sql = o.str();	
		rc = stmt.prepare(sql.c_str(), sql.length()) && stmt.bindParam(params) && stmt.exec();
		CHECK_RETURN(rc, 0, "execute sql: %s error", sql.c_str());
		Debug("insertTable: %s, sql: %s", table.c_str(), sql.c_str());

		return this->_dbhandler->insertId();
	}

#if 0	
	//
	// insert or update entity to db
	bool EasydbInternal::insertOrUpdate(std::string table, const Entity* entity) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		CHECK_RETURN(this->_tables.find(table) != this->_tables.end(), false, "table: %s not exist", table.c_str());
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];
		
		std::ostringstream sql_fields, sql_insert, sql_update;
		sql_fields << "id";
		sql_insert << entity->id;
		sql_update << "`id`=" << entity->id;
		for (auto& iterator : entity->values) {
			const Entity::Value& value = iterator.second;
			CHECK_RETURN(valid_type(value.type), false, "illegal ValueType: %d", value.type);		
			enum_field_types field_type = convert(value);
		
			// add new field
			if (desc_fields.find(iterator.first) == desc_fields.end()) {
				bool rc = this->addField(table, iterator.first, field_type);
				CHECK_RETURN(rc, false, "add new field: %s, type: %d error", iterator.first.c_str(), value.type);
				this->loadFieldDescriptor(table);//NOTE: dont reload field descriptor
			}
		
			const FieldDescriptor& fieldDescriptor = desc_fields[iterator.first];
			CHECK_RETURN(fieldDescriptor.type == field_type, false, "fieldDescriptor.type: %d, field_type: %d", fieldDescriptor.type, field_type);
					
			sql_fields << ",`" << iterator.first << "`";
			sql_insert << ","; sql_update << ",";
		
			switch (value.type) {
				case Entity::Value::type_integer: 
					sql_insert << value.value_integer;
					sql_update << "`" << iterator.first << "`=" << value.value_integer;
					break;
		
				case Entity::Value::type_float:
					sql_insert << value.value_float;
					sql_update << "`" << iterator.first << "`=" << value.value_float;
					break;
		
				case Entity::Value::type_bool:
					sql_insert << value.value_bool;
					sql_update << "`" << iterator.first << "`=" << value.value_bool;
					break;
		
				case Entity::Value::type_string:
					sql_insert << "'" << value.value_string << "'";
					sql_update << "`" << iterator.first << "`=" << "'" << value.value_string << "'";
					break;
		
				default: CHECK_RETURN(false, false, "illegal value type: %d", value.type); break;
			}
		}
		
		std::ostringstream sql;
		sql << "INSERT INTO `" << table << "` (" << sql_fields.str() << ") VALUES (" << sql_insert.str();
		sql << ") ON DUPLICATE KEY UPDATE " << sql_update.str();
		Debug("serialize sql: %s", sql.str().c_str());
		return this->_dbhandler->runCommand(sql.str());	
	}
#endif	

	//
	// load entity from db
	bool EasydbInternal::retrieve(std::string table, uint64_t entityid, Entity* entity) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		//
		// check table
		CHECK_RETURN(this->_tables.find(table) != this->_tables.end(), false, "table: %s not exist", table.c_str());
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];

		//
		// NOTE: include `ID` field
		std::ostringstream sql;
		sql << "SELECT * FROM `" << table << "` WHERE ID = " << entityid;
		
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
			CHECK_RETURN(desc_fields.find(field.org_name) != desc_fields.end(), false, "org_name: %s not exist, table: %s", field.org_name, table.c_str());
			CHECK_RETURN(valid_type(field.type), false, "illegal field.type: %d", field.type);
			bool is_unsigned = IS_UNSIGNED(field.flags);
			try {
				fieldvalue(field.type, field.org_name, entity, row[i], is_unsigned);
			} 
			catch(std::exception& e) {
				Error("field: %s, type: %d, convert except: %s", field.org_name, field.type, e.what());
			}
		}
		SafeDelete(result);
		
		return true;
	}

	//
	// dump all of tables
	void EasydbInternal::dumpTables() {
		Debug("current database: %s", this->_database.c_str());
		for (auto& i : this->_tables) {
			Debug("    table: %s", i.first.c_str());
			std::unordered_map<std::string, FieldDescriptor>& desc_fields = i.second;
			for (auto& iterator : desc_fields) {
				FieldDescriptor& descriptor = iterator.second;
				Debug("        %s(%d):%s\n", iterator.first.c_str(), descriptor.length, fieldstring(descriptor.type));
			}
		}
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

		this->dumpTables();
		return true;
	}

	//
	// load special field descriptor of table
	bool EasydbInternal::loadFieldDescriptor(std::string table) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		std::unordered_map<std::string, FieldDescriptor>& desc_fields = this->_tables[table];
		desc_fields.clear();

		//
		// NOTE: include `ID` field
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
		sql << "CREATE TABLE `" << table << "` (`ID` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY ";
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
	// alter old field
	bool EasydbInternal::alterField(std::string table, const std::string& field_name, enum_field_types field_type) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		CHECK_RETURN(m2string.find(field_type) != m2string.end(), false, "illegal field type: %d", field_type); 	
		std::ostringstream sql;
		sql << "ALTER TABLE `" << table << "` MODIFY `" << field_name << "` " << m2string[field_type] << " NOT NULL";
		Debug("alter table: %s, field: %s, to type: %s", table.c_str(), field_name.c_str(), m2string[field_type]);
		return this->dbhandler->runCommand(sql.str());
	}

	//
	// delete entity
	bool EasydbInternal::deleteEntity(std::string table, uint64_t entityid) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		//
		// remove cache
		
		std::ostringstream sql;
		sql << "DELETE FROM `" << table << "` WHERE `ID` = " << entityid;
		return this->_dbhandler->runCommand(sql.str());
	}
}


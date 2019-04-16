/*
 * \file: EasydbInternal.cpp
 * \brief: Created by hushouguo at 14:19:44 Apr 15 2019
 */

#include "tnode.h"
#include "mysql/MySQLResult.h"
#include "mysql/MySQLStatement.h"
#include "mysql/MySQL.h"
#include "Easydb.h"
#include "EasydbInternal.h"

BEGIN_NAMESPACE_TNODE {
	EasydbInternal::EasydbInternal() {
		//this->_tree.MapPath("", "./");
		//this->_in = new Importer(&this->_tree, &this->_errorCollector);
	}

	Easydb::~Easydb() {}
	EasydbInternal::~EasydbInternal() {
		this->stop();
	}

	void EasydbInternal::stop() {
		if (!this->_isstop) {
			this->_isstop = true;
			if (this->_dbhandler) {
				//TODO: flush dirty entity to db
				this->_dbhandler->closeDatabase();
			}
			SafeDelete(this->_dbhandler);
			/*
			SafeDelete(this->_in);
			for (auto& i : this->_messages) {
				Message* message = i.second;
				SafeDelete(message);
			}
			this->_messages.clear();
			*/
		}
	}
	
	bool EasydbInternal::connectServer(std::string host, std::string user, std::string passwd, int port) {
		this->_dbhandler = new MySQL();
		bool rc = this->_dbhandler->openDatabase(
			host.c_str(), 
			user.c_str(), 
			passwd.c_str(), 
			nullptr, 
			port);
		CHECK_RETURN(rc, false, "connectServer(%s:%s:%s:%d) error", 
			host.c_str(), 
			user.c_str(), 
			passwd.c_str(), 
			port);
		return !(this->_isstop = false);
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
 	
	bool EasydbInternal::createObject(std::string table, Object* object) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
        //
        // check if table exists
        if (this->_objects.find(table) == this->_objects.end()) {
        	bool rc = this->createTable(table);
        	CHECK_RETURN(rc, 0, "create table: %s error", table.c_str());
        }

		std::unordered_map<u64, Object*>& objects = this->_objects[table];
		CHECK_RETURN(object->id == 0 || objects.find(object->id) == objects.end(), false, "duplicate object.id: 0x%lx", object->id);

		//
		// insert object into table
		bool rc = this->insertObject(table, object);
		CHECK_RETURN(rc, 0, "insert object to table: %s error", table.c_str());
		CHECK_RETURN(objects.find(object->id) == objects.end(), 0, "duplicate entityid: 0x%lx", object->id);

		//
		// add object to cache
		return objects.insert(std::make_pair(object->id, object)).second;
	}
	
	Object* EasydbInternal::retrieveObject(std::string table, uint64_t id) {
        CHECK_RETURN(this->_dbhandler, nullptr, "not connectServer");
        CHECK_RETURN(this->_objects.find(table) != this->_objects.end(), nullptr, "table: %s not exist", table.c_str());

		//
		// check cache
		std::unordered_map<u64, Object*>& objects = this->_objects[table];
		auto iterator = objects.find(id);
		if (iterator != objects.end()) {
			return iterator->second;
		}

		//
		// create new object
		Object* object = new Object(id);
		bool rc = this->retrieveObject(table, object);
		if (!rc) {
			SafeDelete(object);
			CHECK_RETURN(false, nullptr, "retrieve object: 0x%lx error", id);
		}

		//
		// add object to cache
		objects.insert(std::make_pair(id, object));		
		
		return object;
	}
	
	bool EasydbInternal::createTable(std::string table) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
        std::ostringstream sql;
        sql << "CREATE TABLE `" << table << "` (`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY ";
        sql << ", `data` LONGBLOB NOT NULL ";
        sql << ") ENGINE=InnoDB DEFAULT CHARSET=utf8";
        return this->_dbhandler->runCommand(sql.str());	
    }
    
	bool EasydbInternal::insertObject(std::string table, Object* object) {
		CHECK_RETURN(this->_dbhandler, 0, "not connectServer");
		//
		// sql statement
		MySQLStatement stmt(this->_dbhandler);
		std::ostringstream o;
		if (object->id == 0) {
			o << "INSERT INTO `" << table << "` (`data`) VALUES (?)";
		}
		else {
			o << "INSERT INTO `" << table << "` (`id`, `data`) VALUES (" << object->id << ", ?)";
		}
		const std::string sql = o.str();
		bool rc = stmt.prepare(sql.c_str(), sql.length());
		CHECK_RETURN(rc, false, "prepare sql: %s error", sql.c_str());

		unsigned long lengths[1] = { object->data.size() };			
		MYSQL_BIND params[1];
		memset(params, 0, sizeof(params));
		params[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
		params[0].buffer = object->data.rbuffer();
		params[0].is_unsigned = true;
		params[0].length = &lengths[0];

		//
		// execute sql
		rc = stmt.bindParam(params) && stmt.exec();
		CHECK_RETURN(rc, false, "execute sql: %s error", sql.c_str());

		if (object->id == 0) {
			object->id = this->_dbhandler->insertId();
		}
		
		return true;
    }

    bool EasydbInternal::retrieveObject(std::string table, Object* object) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		//
		// sql statement
		MySQLStatement stmt(this->_dbhandler);
		std::ostringstream o;
		o << "SELECT data FROM `" << table << "` WHERE id = " << object->id;
		const std::string sql = o.str();
		bool rc = stmt.prepare(sql.c_str(), sql.length());
		CHECK_RETURN(rc, false, "prepare sql: %s error", sql.c_str());
		rc = stmt.exec();
		CHECK_RETURN(rc, false, "execute sql: %s error", sql.c_str());

		//
		// bind result
		unsigned long lengths[1] = {0};
		MYSQL_BIND result[1];
		memset(result, 0, sizeof(result));
		
		result[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
		result[0].buffer = nullptr;
		result[0].buffer_length = 0;
		result[0].length = &lengths[0];
			
		if (!stmt.bindResult(result)) { return false; }
		if (!stmt.fetch()) {		
			//if (stmt.lastError() != MYSQL_DATA_TRUNCATED) {
			//	return false;
			//}
		
			CHECK_RETURN(lengths[0] != 0 && stmt.lastError() != CR_NO_DATA, false, "sql:%s, fetch lengths is zero", sql.c_str());
			CHECK_RETURN(stmt.lastError() == 0, false, "sql:%s, fetch error: %d", stmt.lastError());
		
			result[0].buffer = object->data.wbuffer(lengths[0]);
			result[0].buffer_length = lengths[0];
			if (!stmt.fetchColumn(&result[0], 0)) {
				return false;
			}
		}
		stmt.freeResult();
		
		object->data.append(lengths[0]);

		return true;
    }
}

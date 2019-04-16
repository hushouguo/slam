/*
 * \file: EasydbInternal.cpp
 * \brief: Created by hushouguo at 14:19:44 Apr 15 2019
 */

#include "tnode.h"
#include "tools/Tools.h"
#include "mysql/MySQLResult.h"
#include "mysql/MySQLStatement.h"
#include "mysql/MySQL.h"
#include "lua/luaT_message_parser.h"
#include "Easydb.h"
#include "EasydbInternal.h"

BEGIN_NAMESPACE_TNODE {
	EasydbInternal::EasydbInternal() {		
		this->_tableParser = luaT_message_parser_creator::create();
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
			SafeDelete(this->_tableParser);
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
		//select database
		bool rc = this->_dbhandler->selectDatabase(database);
		CHECK_RETURN(rc, false, "select database: %s error", database.c_str());
		//load tables
		std::set<std::string> tables;
		rc = this->_dbhandler->loadTable("%", tables);
		CHECK_RETURN(rc, false, "load table from database: %s error", database.c_str());
		for (auto& table : tables) {
			auto& objects = this->_objects[table];
			Debug << "load table: " << table << ", " << objects.size();
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

	u64 EasydbInternal::createObject(std::string table, u64 id, google::protobuf::Message* message) {
        CHECK_RETURN(this->_dbhandler, 0, "not connectServer");
        //
        // check if table exists
        if (this->_objects.find(table) == this->_objects.end()) {
        	bool rc = this->createTable(table);
        	CHECK_RETURN(rc, 0, "create table: %s error", table.c_str());
        }

		auto& objects = this->_objects[table];
		CHECK_RETURN(id == 0 || objects.find(id) == objects.end(), false, "duplicate object.id: 0x%lx, table: %s", id, table.c_str());

		//
		// serialize protobuf::Message to buffer
		ByteBuffer buffer;
		size_t byteSize = message->ByteSize();
		bool rc = message->SerializeToArray(buffer.wbuffer(byteSize), byteSize);
		CHECK_RETURN(rc, 0, "Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
		buffer.append(byteSize);

		//
		// insert object into table
		u64 objectid = this->addObject(table, id, &buffer);
		CHECK_RETURN(objectid != 0, 0, "insert object: 0x%lx to table: %s error", id, table.c_str());
		CHECK_RETURN(objects.find(objectid) == objects.end(), 0, "duplicate entityid: 0x%lx", objectid);

		//
		// add object to cache
		objects[objectid] = message;
		return objectid;
	}
		
	google::protobuf::Message* EasydbInternal::retrieveObject(std::string table, u64 id) {
        CHECK_RETURN(this->_dbhandler, nullptr, "not connectServer");
        CHECK_RETURN(this->_objects.find(table) != this->_objects.end(), nullptr, "table: %s not exist", table.c_str());

		//
		// check cache
		auto& objects = this->_objects[table];
		auto iterator = objects.find(id);
		if (iterator != objects.end()) { return iterator->second; }

		//
		// fetch buffer from db
		ByteBuffer buffer;
		bool rc = this->getObject(table, id, &buffer);
		CHECK_RETURN(rc, nullptr, "getObject: 0x%lx from table: %s error", id, table.c_str());

		u32 msgid = hashString(table.c_str());
		
		//
		// decode buffer to protobuf::Message
		google::protobuf::Message* message = this->tableParser()->decode(msgid, buffer.rbuffer(), buffer.size());
		CHECK_RETURN(message, nullptr, "decode buffer failure: %d", msgid);						

		//
		// add object to cache
		objects[id] = message;
		
		return message;
	}
	
	bool EasydbInternal::deleteObject(std::string table, u64 id) {
		//TODO:
		return false;
	}
	
	bool EasydbInternal::updateObject(std::string table, u64 id, google::protobuf::Message*) {
		//TODO:
		return false;
	}
 	
	bool EasydbInternal::createTable(std::string table) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
        std::ostringstream sql;
        sql << "CREATE TABLE `" << table << "` (`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY ";
        sql << ", `data` LONGBLOB NOT NULL ";
        sql << ") ENGINE=InnoDB DEFAULT CHARSET=utf8";
        return this->_dbhandler->runCommand(sql.str());	
    }

    //
    // insert object to db
	u64 EasydbInternal::addObject(std::string table, u64 id, const ByteBuffer* buffer) {
		CHECK_RETURN(this->_dbhandler, 0, "not connectServer");
        CHECK_RETURN(this->_objects.find(table) != this->_objects.end(), 0, "table: %s not exist", table.c_str());
        auto& objects = this->_objects[table];
        CHECK_RETURN(id == 0 || objects.find(id) == objects.end(), 0, "already exist object: 0x%lx, table: %s", id, table.c_str());
				
		//
		// sql statement
		MySQLStatement stmt(this->_dbhandler);
		std::ostringstream o;
		if (id == 0) {	// id == 0 means auto increment
			o << "INSERT INTO `" << table << "` (`data`) VALUES (?)";
		}
		else {
			o << "INSERT INTO `" << table << "` (`id`, `data`) VALUES (" << id << ", ?)";
		}
		
		const std::string sql = o.str();
		bool rc = stmt.prepare(sql.c_str(), sql.length());
		CHECK_RETURN(rc, 0, "prepare sql: %s error", sql.c_str());

		//
		// check buffer is empty
		CHECK_ALARM(buffer->size() > 0, "object: 0x%lx buffer is empty", id);

		unsigned long lengths[1] = { buffer->size() };
		MYSQL_BIND params[1];
		memset(params, 0, sizeof(params));
		params[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
		params[0].buffer = buffer->rbuffer();
		params[0].is_unsigned = true;
		params[0].length = &lengths[0];

		//
		// execute sql
		rc = stmt.bindParam(params) && stmt.exec();
		CHECK_RETURN(rc, 0, "execute sql: %s error", sql.c_str());

		if (id == 0) {
			id = this->_dbhandler->insertId();
		}

		return id;
    }

	//
	// fetch object from db
    bool EasydbInternal::getObject(std::string table, u64 id, ByteBuffer* buffer) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
        CHECK_RETURN(this->_objects.find(table) != this->_objects.end(), false, "table: %s not exist", table.c_str());
        
		//
		// sql statement
		MySQLStatement stmt(this->_dbhandler);
		std::ostringstream o;
		o << "SELECT data FROM `" << table << "` WHERE id = " << id;
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

		//
		// fetch result to buffer
		if (!stmt.bindResult(result)) { return false; }
		if (!stmt.fetch()) {		
			//if (stmt.lastError() != MYSQL_DATA_TRUNCATED) {
			//	return false;
			//}
		
			CHECK_RETURN(lengths[0] != 0 && stmt.lastError() != CR_NO_DATA, false, "sql:%s, fetch lengths is zero", sql.c_str());
			CHECK_RETURN(stmt.lastError() == 0, false, "sql:%s, fetch error: %d", stmt.lastError());
		
			result[0].buffer = buffer->wbuffer(lengths[0]);
			result[0].buffer_length = lengths[0];
			if (!stmt.fetchColumn(&result[0], 0)) {
				return false;
			}
		}
		stmt.freeResult();
		
		buffer->append(lengths[0]);
		return true;
    }
}

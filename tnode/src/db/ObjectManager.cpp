/*
 * \file: ObjectManager.cpp
 * \brief: Created by hushouguo at 14:37:04 Apr 24 2019
 */

#include "tnode.h"
#include "tools/Tools.h"
#include "tools/Registry.h"
#include "tools/Singleton.h"
#include "xml/XmlParser.h"
#include "config/Config.h"
#include "message/MessageParser.h"
#include "mysql/MySQLResult.h"
#include "mysql/MySQLStatement.h"
#include "mysql/MySQL.h"
#include "lua/luaT_message_parser.h"
#include "db/Easydb.h"
#include "db/EasydbInternal.h"
#include "db/ObjectManager.h"

BEGIN_NAMESPACE_TNODE {
	u64 ObjectManager::createObject(EasydbInternal* easydb, std::string table, u64 id, Message* message) {
		assert(easydb);
		assert(easydb->dbhandler());

        //
        // check if table exists
        if (this->_objects.find(table) == this->_objects.end()) {
        	bool rc = this->CreateTable(easydb, table);
        	CHECK_RETURN(rc, 0, "create table: %s error", table.c_str());
        }

		//
		// check if object exists
		auto& objects = this->_objects[table];
		CHECK_RETURN(id == 0 || objects.find(id) == objects.end(), 0, "duplicate object.id: %ld, table: %s", id, table.c_str());

		//
		// serialize protobuf::Message to buffer
		ByteBuffer buffer;
		size_t byteSize = message->ByteSize();
		bool rc = message->SerializeToArray(buffer.wbuffer(byteSize), byteSize);
		CHECK_RETURN(rc, 0, "Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
		buffer.wlength(byteSize);

		//
		// insert object into table
		u64 objectid = this->InsertObjectToTable(easydb, table, id, &buffer);
		CHECK_RETURN(objectid != 0, 0, "insert object: %ld to table: %s error", id, table.c_str());

		//
		// check objectid if exists
		if (true) {
			SpinlockerGuard guard(&this->_locker);
			bool exist_objectid = objects.find(objectid) == objects.end();
			CHECK_RETURN(!exist_objectid, 0, "duplicate entityid: %ld", objectid);
		}

		//
		// create new object
		db_object* object = new db_object(easydb, objectid, false, message);

		//
		// add object to cache
		SpinlockerGuard guard(&this->_locker);
		objects[objectid] = object;

		return objectid;
	}

	Message* ObjectManager::retrieveObject(EasydbInternal* easydb, std::string table, u64 id) {
		assert(easydb);
		assert(easydb->dbhandler());

		auto& objects = this->_objects[table];
		
		//
		// check cache if exists
		if (true) {
			SpinlockerGuard guard(&this->_locker);			
			auto iterator = objects.find(id);
			if (iterator != objects.end()) { 
				db_object* object = iterator->second;
				assert(object);
				assert(object->message);
				return object->message;
			}
		}

		//
		// fetch buffer from db
		ByteBuffer buffer;
		bool rc = this->GetObjectFromTable(easydb, table, id, &buffer);
		CHECK_RETURN(rc, nullptr, "getObject: %ld from table: %s error", id, table.c_str());

		u32 msgid = hashString(table.c_str());
		
		//
		// decode buffer to NEW protobuf::Message
		Message* message = easydb->tableParser()->DecodeToMessage(msgid, buffer.rbuffer(), buffer.size());
		CHECK_RETURN(message, nullptr, "decode buffer failure: %d", msgid);						

		//
		// create new object
		db_object* object = new db_object(easydb, id, false, message);
		
		//
		// add object to cache
		SpinlockerGuard guard(&this->_locker);
		objects[id] = object;
		
		return message;
	}

	bool ObjectManager::deleteObject(EasydbInternal* easydb, std::string table, u64 id) {
		assert(easydb);
		assert(easydb->dbhandler());

        auto& objects = this->_objects[table];
        
		//
		// remove cache if exists
		if (true) {
			SpinlockerGuard guard(&this->_locker);
	        auto iterator = objects.find(id);
	        if (iterator != objects.end()) {
				db_object* object = iterator->second;
				SafeDelete(object->message);
				SafeDelete(object);
	        	objects.erase(iterator);
	        }
        }

		return this->DeleteObjectFromTable(easydb, table, id);
	}

	bool ObjectManager::updateObject(EasydbInternal* easydb, std::string table, u64 id, Message* update_msg) {
		assert(easydb);
		assert(easydb->dbhandler());
		
		auto& objects = this->_objects[table];

		db_object* object = nullptr;
		if (true) {
			//
			// find object from cache
			this->_locker.lock();
			auto iterator = objects.find(id);
			if (iterator != objects.end()) {
				object = iterator->second;
				this->_locker.unlock();
			}
			else {
				this->_locker.unlock();
				
				Alarm("updateObject: %ld not cache, table: %s", id, table.c_str());
				this->retrieveObject(table, id);
				
				//
				// find object again
				SpinlockerGuard guard(&this->_locker);
				iterator = objects.find(id);
				CHECK_RETURN(iterator != objects.end(), false, "updateObject: %ld error, table: %s", id, table.c_str());
				object = iterator->second;
			}
		}

		assert(object);
		assert(object->message);

		bool rc = easydb->tableParser()->MergeMessage(object->message, update_msg);
		CHECK_RETURN(rc, false, "merge message error: %ld, table: %s", id, table.c_str());

		return object->dirty = true;
	}

	bool ObjectManager::flushObject(EasydbInternal* easydb, std::string table, u64 id) {
		assert(easydb);
		assert(easydb->dbhandler());

		auto& objects = this->_objects[table];

		//
		// check object if exist
		db_object* object = nullptr;
		if (true) {
			SpinlockerGuard guard(&this->_locker);
			auto iterator = objects.find(id);
			if (iterator != objects.end()) {
				object = iterator->second;
			}
		}
		
		CHECK_RETURN(object, false, "not found object: %ld, table: %s in cache", id, table.c_str());

		assert(object);
		assert(object->message);
		assert(object->id == id);
		if (!object->dirty) {
			return true; // async off or already flush
		}
		//CHECK_ALARM(object->dirty, true, "object: %ld, table: %s not dirty", id, table.c_str());
		
		bool rc = this->FlushObjectToTable(table, object);
		if (rc) {
			object->dirty = false;
		}

		return rc;
	}

	//-----------------------------------------------------------------------------------------------------
	//
	bool ObjectManager::CreateTable(EasydbInternal* easydb, std::string table) {
		assert(easydb);
		assert(easydb->dbhandler());
        std::ostringstream sql;
        sql << "CREATE TABLE `" << table << "` (`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY ";
        sql << ", `data` LONGBLOB NOT NULL ";
        sql << ") ENGINE=InnoDB DEFAULT CHARSET=utf8";
        return easydb->dbhandler()->runCommand(sql.str());	
	}

	u64  ObjectManager::InsertObjectToTable(EasydbInternal* easydb, std::string table, u64 id, const ByteBuffer* buffer) {
		assert(easydb);
		assert(easydb->dbhandler());

		//
		// check buffer is empty
		CHECK_ALARM(buffer->size() > 0, "object: %ld buffer is empty", id);

		//
		// sql statement
		MySQLStatement stmt(easydb->dbhandler());
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
			id = easydb->dbhandler()->insertId();
		}

		return id;
	}

	bool ObjectManager::GetObjectFromTable(EasydbInternal* easydb, std::string table, u64 id, ByteBuffer* buffer) {
		assert(easydb);
		assert(easydb->dbhandler());
        
		//
		// sql statement
		MySQLStatement stmt(easydb->dbhandler());
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
		
		buffer->wlength(lengths[0]);
		return true;
	}

	bool ObjectManager::DeleteObjectFromTable(EasydbInternal* easydb, std::string table, u64 id) {
		assert(easydb);
		assert(easydb->dbhandler());

        //
        // remove object from db
		std::ostringstream sql;
		sql << "DELETE FROM `" << table << "` WHERE id = " << id;
		return easydb->dbhandler()->runCommand(sql.str());
	}

	bool ObjectManager::FlushObjectToTable(EasydbInternal* easydb, std::string table, db_object* object) {
		assert(easydb);
		assert(easydb->dbhandler());

		//
		// serialize protobuf::Message to buffer
		ByteBuffer buffer;
		size_t byteSize = object->message->ByteSize();
		bool rc = object->message->SerializeToArray(buffer.wbuffer(byteSize), byteSize);
		CHECK_RETURN(rc, false, "Serialize message:%s failure, byteSize:%ld", object->message->GetTypeName().c_str(), byteSize);
		buffer.wlength(byteSize);
		
		//
		// flush buffer to db
		return this->FlushBufferToTable(easydb, table, object->id, &buffer);		
	}

	bool ObjectManager::FlushBufferToTable(EasydbInternal* easydb, std::string table, u64 id, const ByteBuffer* buffer) {
		assert(easydb);
		assert(easydb->dbhandler());

		//
		// sql statement
		MySQLStatement stmt(easydb->dbhandler());
		std::ostringstream o;
		o << "UPDATE `" << table << "` SET `data` = ? WHERE id = " << id;
		
		const std::string sql = o.str();
		bool rc = stmt.prepare(sql.c_str(), sql.length());
		CHECK_RETURN(rc, 0, "prepare sql: %s error", sql.c_str());

		//
		// check buffer is empty
		CHECK_ALARM(buffer->size() > 0, "object: %ld buffer is empty", id);

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

		return true;
	}

	void ObjectManager::FlushAll(bool cleanup) {
		//
		// flush dirty entity to db & release all of db_objects
		for (auto& i : this->_objects) {
			const std::string& table = i.first;
			auto& objects = i.second;
			for (auto& iterator : objects) {
				db_object* object = iterator.second;
				assert(object);
				assert(object->message);
				assert(object->id == iterator.first);
				if (object->dirty) {
					object->dirty = !this->FlushObjectToTable(object->easydb, table, object);
					Debug("flush object: %ld, table:%s [%s]", object->id, table.c_str(), object->dirty ? "FAIL" : "OK");
				}
				if (cleanup) {
					SafeDelete(object->message);
					SafeDelete(object);
				}
			}
		}
		if (cleanup) {
			this->_objects.clear();
		}
	}

	INITIALIZE_INSTANCE(ObjectManager);
}



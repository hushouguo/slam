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
		// check if object exists
		CHECK_RETURN(id == 0 || this->GetObjectFromCache(id) == nullptr, 0, "duplicate object.id: %ld, table: %s", id, table.c_str());

		//
		// create new object
		db_object* object = new db_object(easydb, table, id, false);

		//
		// serialize protobuf::Message to buffer
		size_t byteSize = message->ByteSize();
		bool rc = message->SerializeToArray(object->data.wbuffer(byteSize), byteSize);
		if (!rc) {
			SafeDelete(object);
			CHECK_RETURN(false, 0, "Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
		}
		object->data.wlength(byteSize);

		//
		// insert object into table
		rc = this->InsertObjectToTable(easydb, table, object);
		if (!rc) {
			SafeDelete(object);
			CHECK_RETURN(false, 0, "insert object: %ld to table: %s error", id, table.c_str());
		}

		//
		// check objectid if exists, objectid maybe is reallocated
		if (this->GetObjectFromCache(object->id) != nullptr) {
			SafeDelete(object);
			CHECK_RETURN(false, 0, "duplicate entityid: %ld", id);
		}
		
		//
		// add object to cache
		this->InsertObjectToCache(object);

		return object->id;
	}


	ByteBuffer* ObjectManager::retrieveObject(EasydbInternal* easydb, std::string table, u64 id) {
		assert(easydb);
		assert(easydb->dbhandler());
		
		//
		// check cache if exists
		db_object* object = this->GetObjectFromCache(id);
		if (object) {
			return &object->data;
		}

		//
		// create new object
		object = new db_object(easydb, table, id, false);

		//
		// get object from db
		bool rc = this->GetObjectFromTable(easydb, table, object);
		CHECK_RETURN(rc, nullptr, "retrieveObject: %ld from table: %s error", id, table.c_str());
				
		//
		// add object to cache
		this->InsertObjectToCache(object);
		
		return &object->data;
	}


	bool ObjectManager::deleteObject(EasydbInternal* easydb, std::string table, u64 id) {
		assert(easydb);
		assert(easydb->dbhandler());
		return this->DeleteObjectFromCache(id) && this->DeleteObjectFromTable(easydb, table, id);
	}


	bool ObjectManager::updateObject(EasydbInternal* easydb, std::string table, u64 id, Message* update_msg) {
		assert(easydb);
		assert(easydb->dbhandler());
		
		//
		// get object from cache
		db_object* object = this->GetObjectFromCache(id);
		if (!object) {
			Alarm("updateObject: %ld not in cache, table: %s", id, table.c_str());
			this->retrieveObject(easydb, table, id);
			//
			// get object from cache again
			object = this->GetObjectFromCache(id);
		}

		CHECK_RETURN(object, false, "updateObject: %ld error, not found object, table: %s", id, table.c_str());

		u32 msgid = hashString(table.c_str(), table.length());
		
		//
		// decode buffer to NEW protobuf::Message
		Message* message = easydb->tableParser()->DecodeToNewMessage(msgid, object->data.rbuffer(), object->data.size());
		CHECK_RETURN(message, nullptr, "decode buffer failure: %d", msgid);

		//
		// merge update_msg to message
		bool rc = easydb->tableParser()->MergeMessage(message, update_msg);
		if (!rc) {
			SafeDelete(message);
			CHECK_RETURN(false, false, "merge message error: %ld, table: %s", id, table.c_str());
		}

		//
		// reset object->data
		object->data.reset();
		
		//
		// serialize protobuf::Message to buffer
		size_t byteSize = message->ByteSize();
		rc = message->SerializeToArray(object->data.wbuffer(byteSize), byteSize);
		if (!rc) {
			SafeDelete(message);
			CHECK_RETURN(false, false, "Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
		}
		object->data.wlength(byteSize);		

		SafeDelete(message);
		return object->dirty = true;
	}


	bool ObjectManager::flushObject(EasydbInternal* easydb, std::string table, u64 id) {
		assert(easydb);
		assert(easydb->dbhandler());

		//
		// get object from cache
		db_object* object = this->GetObjectFromCache(id);
		CHECK_RETURN(object, false, "not found object: %ld, table: %s in cache", id, table.c_str());

		if (!object->dirty) {
			return true; // perhaps already flush
		}

		bool rc = this->FlushObjectToTable(easydb, table, object);
		if (rc) {
			object->dirty = false;
		}

		return rc;
	}


	void ObjectManager::FlushAll(EasydbInternal* easydb, bool cleanup) {
		SpinlockerGuard guard(&this->_locker);
		
		//
		// flush dirty entity to db & release all of db_objects
		std::vector<u64> v;
		for (auto& i : this->_objects) {
			db_object* object = i.second;
			assert(object);
			assert(object->id == i.first);
			if (object->easydb == easydb) {
				v.push_back(object->id);
			}
		}

		for (auto id : v) {
			db_object* object = this->_objects[id];
			if (object->dirty) {
				object->dirty = !this->FlushObjectToTable(object->easydb, object->table, object);
				Debug("flush object: %ld, table:%s [%s]", object->id, object->table.c_str(), object->dirty ? "FAIL" : "OK");
			}
			
			if (cleanup) {
				SafeDelete(object);
				auto i = this->_objects.find(id);
				assert(i != this->_objects.end());
				this->_objects.erase(i);
			}
		}
	}


	//-----------------------------------------------------------------------------------------------------
	//

	bool ObjectManager::InsertObjectToCache(db_object* object) {
		SpinlockerGuard guard(&this->_locker);
		return this->_objects.insert(std::make_pair(object->id, object)).second;
	}
	
	db_object* ObjectManager::GetObjectFromCache(u64 id) {
		SpinlockerGuard guard(&this->_locker);
		auto iterator = this->_objects.find(id);
		return iterator != this->_objects.end() ? iterator->second : nullptr;
	}
	
	bool ObjectManager::DeleteObjectFromCache(u64 id) {
		SpinlockerGuard guard(&this->_locker);
		auto iterator = this->_objects.find(id);
		if (iterator != this->_objects.end()) {
			db_object* object = iterator->second;
			SafeDelete(object);
			this->_objects.erase(iterator);
			return true;
		}
		return false;
	}

	//-----------------------------------------------------------------------------------------------------
	//
	bool ObjectManager::InsertObjectToTable(EasydbInternal* easydb, std::string table, db_object* object) {
		assert(easydb);
		assert(easydb->dbhandler());

		ByteBuffer& buffer = object->data;
		
		//
		// check buffer is empty
		CHECK_ALARM(buffer.size() > 0, "object: %ld buffer is empty", object->id);

		//
		// sql statement
		MySQLStatement stmt(easydb->dbhandler());
		std::ostringstream o;
		if (object->id == 0) {	// id == 0 means auto increment
			o << "INSERT INTO `" << table << "` (`data`) VALUES (?)";
		}
		else {
			o << "INSERT INTO `" << table << "` (`id`, `data`) VALUES (" << object->id << ", ?)";
		}
		
		const std::string sql = o.str();
		bool rc = stmt.prepare(sql.c_str(), sql.length());
		CHECK_RETURN(rc, false, "prepare sql: %s error", sql.c_str());

		unsigned long lengths[1] = { buffer.size() };
		MYSQL_BIND params[1];
		memset(params, 0, sizeof(params));
		params[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
		params[0].buffer = buffer.rbuffer();
		params[0].is_unsigned = true;
		params[0].length = &lengths[0];

		//
		// execute sql
		rc = stmt.bindParam(params) && stmt.exec();
		CHECK_RETURN(rc, false, "execute sql: %s error", sql.c_str());
		
		if (object->id == 0) {
			object->id = easydb->dbhandler()->insertId();
		}

		return true;
	}

	bool ObjectManager::GetObjectFromTable(EasydbInternal* easydb, std::string table, db_object* object) {
		assert(easydb);
		assert(easydb->dbhandler());

        CHECK_ALARM(object->data.size() == 0, "object: %ld not empty when retrieve", object->id);
        
		//
		// sql statement
		MySQLStatement stmt(easydb->dbhandler());
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

		//
		// fetch result to buffer
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
		
		object->data.wlength(lengths[0]);
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
		// sql statement
		MySQLStatement stmt(easydb->dbhandler());
		std::ostringstream o;
		o << "UPDATE `" << table << "` SET `data` = ? WHERE id = " << object->id;
		
		const std::string sql = o.str();
		bool rc = stmt.prepare(sql.c_str(), sql.length());
		CHECK_RETURN(rc, 0, "prepare sql: %s error", sql.c_str());

		//
		// check buffer is empty
		CHECK_ALARM(object->data.size() > 0, "object: %ld buffer is empty", object->id);

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
		CHECK_RETURN(rc, 0, "execute sql: %s error", sql.c_str());

		return true;
	}

	INITIALIZE_INSTANCE(ObjectManager);
}



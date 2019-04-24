/*
 * \file: EasydbInternal.cpp
 * \brief: Created by hushouguo at 14:19:44 Apr 15 2019
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
	EasydbInternal::EasydbInternal() {		
		this->_tableParser = new MessageParser();
	}

	Easydb::~Easydb() {}
	EasydbInternal::~EasydbInternal() {
		this->stop();
	}

	void EasydbInternal::stop() {
		if (!this->_isstop) {
			this->_isstop = true;
			
			//
			// release MySQL handler
			if (this->_dbhandler) {
				//
				// flush dirty entity to db & release all of db_objects
				this->flushall(true);

				//
				// close MySQL handler
				this->_dbhandler->closeDatabase();
			}
			SafeDelete(this->_dbhandler);

			//
			// release MessageParser
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
		//flush all of dirty
		if (!this->_database.empty()) {
			this->flushall(true);
		}
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

	u64 EasydbInternal::createObject(std::string table, u64 id, Message* message) {
        CHECK_RETURN(this->_dbhandler, 0, "not connectServer");
		return sObjectManager.createObject(this, table, id, message);
	}
		
	Message* EasydbInternal::retrieveObject(std::string table, u64 id) {
        CHECK_RETURN(this->_dbhandler, nullptr, "not connectServer");
        CHECK_RETURN(this->_objects.find(table) != this->_objects.end(), nullptr, "table: %s not exist", table.c_str());
		return sObjectManager.retrieveObject(this, table, id);
	}
	
	bool EasydbInternal::deleteObject(std::string table, u64 id) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
        CHECK_RETURN(this->_objects.find(table) != this->_objects.end(), false, "table: %s not exist", table.c_str());	
		return sObjectManager.deleteObject(this, table, id);
	}
	
	bool EasydbInternal::updateObject(std::string table, u64 id, Message* update_msg) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
        CHECK_RETURN(this->_objects.find(table) != this->_objects.end(), false, "table: %s not exist", table.c_str());
		return sObjectManager.updateObject(this, table, id, update_msg);
	}

 	bool EasydbInternal::flushObject(std::string table, u64 id) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
        CHECK_RETURN(this->_objects.find(table) != this->_objects.end(), false, "table: %s not exist", table.c_str());	
    	return sObjectManager.flushObject(this, table, id);    
 	}
}

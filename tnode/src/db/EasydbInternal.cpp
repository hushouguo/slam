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
				sObjectManager.FlushAll(true);

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

	bool EasydbInternal::selectDatabase(std::string database) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		//
		// reselect maybe cause data confusion
		CHECK_RETURN(this->_database.empty(), false, "reselect database is not allow!");
		bool rc = this->_dbhandler->selectDatabase(database);
		CHECK_RETURN(rc, false, "select database: %s error", database.c_str());
		this->_database = database;
		return true;
	}

	bool EasydbInternal::createTable(std::string table) {
		CHECK_RETURN(this->_dbhandler, false, "not connectServer");
        std::ostringstream sql;
        sql << "CREATE TABLE IF NOT EXISTS `" << table << "` (`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY ";
        sql << ", `data` LONGBLOB NOT NULL ";
        sql << ") ENGINE=InnoDB DEFAULT CHARSET=utf8";
        return this->_dbhandler->runCommand(sql.str());	
	}

	u64 EasydbInternal::createObject(std::string table, u64 id, Message* message) {
        CHECK_RETURN(this->_dbhandler, 0, "not connectServer");
		return sObjectManager.createObject(this, table, id, message);
	}
		
	Message* EasydbInternal::retrieveObject(std::string table, u64 id) {
        CHECK_RETURN(this->_dbhandler, nullptr, "not connectServer");
		return sObjectManager.retrieveObject(this, table, id);
	}
	
	bool EasydbInternal::deleteObject(std::string table, u64 id) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		return sObjectManager.deleteObject(this, table, id);
	}
	
	bool EasydbInternal::updateObject(std::string table, u64 id, Message* update_msg) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
		return sObjectManager.updateObject(this, table, id, update_msg);
	}

 	bool EasydbInternal::flushObject(std::string table, u64 id) {
        CHECK_RETURN(this->_dbhandler, false, "not connectServer");
    	return sObjectManager.flushObject(this, table, id);    
 	}
}

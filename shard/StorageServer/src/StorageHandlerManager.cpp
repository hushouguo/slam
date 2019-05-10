/*
 * \file: StorageHandlerManager.cpp
 * \brief: Created by hushouguo at 10:14:34 May 07 2019
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
#include "MessageStatement.h"
#include "StorageHandler.h"
#include "StorageHandlerManager.h"

BEGIN_NAMESPACE_SLAM {
	bool StorageHandlerManager::init() {
		XmlParser xmlParser;
		if (!xmlParser.open("conf/db.xml")) {
			return false;
		}
		
		XmlParser::XML_NODE root = xmlParser.getRootNode();
		CHECK_RETURN(root, false, "not found root node");
		
		bool rc = true;
		for (XmlParser::XML_NODE node = xmlParser.getChildNode(root, "mysql"); 
				node && rc; node = xmlParser.getNextNode(node, "mysql")) {
				
			int shard = xmlParser.getValueByInteger(node, "shard", 0);
			std::string host = xmlParser.getValueByString(node, "host", "");
			int port = xmlParser.getValueByInteger(node, "port", 0);
			std::string user = xmlParser.getValueByString(node, "user", "");
			std::string password = xmlParser.getValueByString(node, "password", "");
			std::string database = xmlParser.getValueByString(node, "database", "");
			CHECK_BREAK(!database.empty(), "unspecial database: %s", database.c_str());

			StorageHandler* storageHandler = new StorageHandler(shard);
			rc = storageHandler->init(host, user, password, database, port);
			if (rc) {
				rc = this->add(storageHandler);
			}
		}
		
		xmlParser.final();
		return rc;	
	}

	u64 StorageHandlerManager::InsertEntityToTable(u32 shard, std::string table, const Entity* entity) {
		StorageHandler* storageHandler = this->find(shard);
		return storageHandler ? storageHandler->InsertEntityToTable(shard, table, entity) : 0;
	}

	bool StorageHandlerManager::RetrieveEntityFromTable(u32 shard, std::string table, u64 entityid, Entity* entity) {
		StorageHandler* storageHandler = this->find(shard);
		return storageHandler ? storageHandler->RetrieveEntityFromTable(shard, table, entityid, entity) : false;
	}
}


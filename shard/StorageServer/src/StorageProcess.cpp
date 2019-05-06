/*
 * \file: StorageProcess.cpp
 * \brief: Created by hushouguo at 09:49:54 May 05 2019
 */

BEGIN_NAMESPACE_SLAM {
	StorageProcess::StorageProcess(int id) : Entry<int>(id) {
		
	}

	bool StorageProcess::init() {
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

			if (FindOrNull(this->_dbs, shard)) {
				rc = false;
				CHECK_BREAK(false, "duplicate shard: %d", shard);
			}

			MySQL* handler = new MySQL();
			rc = handler->openDatabase(
				host.c_str(),
				user.c_str(),
				password.c_str(),
				nullptr,
				port
			);
			CHECK_BREAK(rc, "connectDatabase(%s:%s:%s:%d) error",
				host.c_str(),
				user.c_str(),
				password.c_str(),
				port
			);
			
			rc = handler->createDatabase(database);
			CHECK_BREAK(rc, "create database: %s error", database.c_str());

			rc = handler->selectDatabase(database);
			CHECK_BREAK(rc, "select database: %s error", database.c_str());

			this->_dbs[shard] = handler;
		}
		
		xmlParser.final();
		return rc;
	}
	
	void StorageProcess::serialize(Easynet* easynet, SOCKET socket, StorageSerializeRequest* request) {
		assert(request->entity().id() == 0 || request->entity().id() == this->id);
		
	}
	
	void StorageProcess::unserialize(Easynet* easynet, SOCKET socket, StorageUnserializeRequest* request) {
		assert(request->entityid() == this->id);
	}
}


/*
 * \file: Easydb.h
 * \brief: Created by hushouguo at 14:17:56 Apr 15 2019
 */
 
#ifndef __EASYDB_H__
#define __EASYDB_H__

#include "tools/Spinlocker.h"
#include "tools/ByteBuffer.h"

BEGIN_NAMESPACE_TNODE {	
	class Easydb {
		public:
			virtual ~Easydb() = 0;

		public:
			virtual bool connectServer(std::string host, std::string user, std::string passwd, int port) = 0;

		public:
			virtual bool createDatabase(std::string) = 0;
			virtual bool selectDatabase(std::string) = 0;
			virtual bool deleteDatabase(std::string) = 0;
			virtual bool findDatabase(std::string) = 0;

		public:
			virtual MessageParser* tableParser() = 0;
			
		public:
			virtual u64 createObject(std::string table, u64 id, Message*) = 0;
			virtual Message* retrieveObject(std::string table, u64 id) = 0;
			virtual bool deleteObject(std::string table, u64 id) = 0;
			virtual bool updateObject(std::string table, u64 id, Message*) = 0;

		public:
			static Easydb* createInstance();
	};
}

#endif

/*
 * \file: Easydb.h
 * \brief: Created by hushouguo at 14:17:56 Apr 15 2019
 */
 
#ifndef __EASYDB_H__
#define __EASYDB_H__

BEGIN_NAMESPACE_TNODE {
	struct Object {
		uint64_t id;
		ByteBuffer data;
		Object(uint64_t _id) : id(_id) {}
	};
	class Easydb {
		public:
			virtual ~Easydb() = 0;
			virtual void stop() = 0;

		public:
			virtual bool connectServer(std::string host, std::string user, std::string passwd, int port) = 0;

		public:
			virtual bool createDatabase(std::string) = 0;
			virtual bool selectDatabase(std::string) = 0;
			virtual bool deleteDatabase(std::string) = 0;
			virtual bool findDatabase(std::string) = 0;

		public:
			virtual bool createObject(std::string table, Object* object) = 0;
			virtual Object* retrieveObject(std::string table, uint64_t id) = 0;

		public:
			static Easydb* createInstance();
	};
}

#endif

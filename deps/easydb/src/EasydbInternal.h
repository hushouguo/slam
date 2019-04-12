/*
 * \file: EasydbInternal.h
 * \brief: Created by hushouguo at 11:24:21 Apr 12 2019
 */
 
#ifndef __EASYDBINTERNAL_H__
#define __EASYDBINTERNAL_H__

namespace db {
	class EasydbInternal : public Easydb {
		public:
			EasydbInternal();
			~EasydbInternal();

		public:
			int connectServer(const char* host, const char* user, const char* passwd, int port) override;
			int connectServer(const char* address, int port) override;

		public:
			bool selectDatabase(std::string) override;

		public:
			bool addEntity(int handle, std::string table, const Entity* entity) override;
			bool modifyEntity(int handle, std::string table, const Entity* entity) override;
			bool loadEntity(int handle, std::string table, uint64_t entityid, Entity* entity) override;
			bool removeEntity(int handle, uint64_t entityid) override;
			bool runQuery(int handle, std::string where, std::vector<Entity*>& entities) override;

		public:
			// MUL KEY
			bool addKey(int handle, std::string table, std::string field) override;
			bool removeKey(int handle, std::string table, std::string field) override;
			// UNI KEY
			bool addUnique(int handle, std::string table, std::string field) override;
			bool removeUnique(int handle, std::string table, std::string field) override;
			// UNSIGNED
			bool addUnsigned(int handle, std::string table, std::string field) override;
			bool removeUnsigned(int handle, std::string table, std::string field) override;

		private:
			MySQL* _dbhandler = nullptr;
	};
}

#endif

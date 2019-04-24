/*
 * \file: EasydbInternal.h
 * \brief: Created by hushouguo at 14:19:17 Apr 15 2019
 */
 
#ifndef __EASYDBINTERNAL_H__
#define __EASYDBINTERNAL_H__

BEGIN_NAMESPACE_TNODE {
	class EasydbInternal : public Easydb {
		public:
			EasydbInternal();
			~EasydbInternal();

		public:
			bool connectServer(std::string host, std::string user, std::string passwd, int port) override;

		public:
			bool createDatabase(std::string) override;
			bool selectDatabase(std::string) override;
			bool createTable(std::string) override;
			
		public:
			u64 createObject(std::string table, u64 id, Message*) override;
			Message* retrieveObject(std::string table, u64 id) override;
			bool deleteObject(std::string table, u64 id) override;
			bool updateObject(std::string table, u64 id, Message*) override;
			bool flushObject(std::string table, u64 id) override;

		public:
			MessageParser* tableParser() override { return this->_tableParser; }
			inline MySQL* dbhandler() { return this->_dbhandler; }

		private:
			bool _isstop = true;
			std::string _database;
			MySQL* _dbhandler = nullptr;
			MessageParser* _tableParser = nullptr;
			void stop();
	};
}

#endif

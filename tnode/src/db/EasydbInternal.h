/*
 * \file: EasydbInternal.h
 * \brief: Created by hushouguo at 14:19:17 Apr 15 2019
 */
 
#ifndef __EASYDBINTERNAL_H__
#define __EASYDBINTERNAL_H__

#define EASYDB_DEF_ASYNC_FLUSH				false
#define EASYDB_DEF_ASYNC_FLUSH_INTERVAL		300u
#define EASYDB_DEF_ASYNC_FLUSH_MAXSIZE		100u

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
			bool deleteDatabase(std::string) override;
			bool findDatabase(std::string) override;
			
		public:
			u64 createObject(std::string table, u64 id, Message*) override;
			Message* retrieveObject(std::string table, u64 id) override;
			bool deleteObject(std::string table, u64 id) override;
			bool updateObject(std::string table, u64 id, Message*) override;
			bool flushObject(std::string table, u64 id) override;

		public:
			MessageParser* tableParser() override { return this->_tableParser; }

		private:
			bool _async_flush = false;
			u32 _async_flush_interval = 0;
			u32 _async_flush_maxsize = 0;
			MessageParser* _tableParser = nullptr;
		
		private:
			bool _isstop = true;
			void stop();
			std::string _database;
			MySQL* _dbhandler = nullptr;
			struct db_object {
				u64 id;
				bool dirty;
				Message* message = nullptr;
				db_object(u64 _id, bool _dirty, Message* _message) : id(_id), dirty(_dirty), message(_message) {}
			};
			Spinlocker _locker;
		    std::unordered_map<std::string, std::unordered_map<u64, db_object*>> _objects;
		    void flushall(bool cleanup);
            			
		private:
		    bool createTable(std::string table);
		    u64  addObject(std::string table, u64 id, const ByteBuffer* buffer);
		    bool getObject(std::string table, u64 id, ByteBuffer* buffer);
		    bool removeObject(std::string table, u64 id);
		    bool setObject(std::string table, u64 id, const ByteBuffer* buffer);
		    bool flushObject(std::string table, db_object* object);
	};
}

#endif

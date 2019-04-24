/*
 * \file: ObjectManager.h
 * \brief: Created by hushouguo at 14:31:50 Apr 24 2019
 */
 
#ifndef __OBJECTMANAGER_H__
#define __OBJECTMANAGER_H__

BEGIN_NAMESPACE_TNODE {
	class EasydbInternal;
	struct db_object {
		EasydbInternal* easydb = nullptr;
		std::string table;
		u64 id;
		bool dirty;
		Message* message = nullptr;
		db_object(EasydbInternal* _easydb, std::string _table, u64 _id, bool _dirty, Message* _message) 
			: easydb(_easydb)
			, table(_table)
			, id(_id)
			, dirty(_dirty)
			, message(_message) 
		{}
	};
	
	class ObjectManager {
		public:
			u64 createObject(EasydbInternal* easydb, std::string table, u64 id, Message*);
			Message* retrieveObject(EasydbInternal* easydb, std::string table, u64 id);
			bool deleteObject(EasydbInternal* easydb, std::string table, u64 id);
			bool updateObject(EasydbInternal* easydb, std::string table, u64 id, Message*);
			bool flushObject(EasydbInternal* easydb, std::string table, u64 id);

		public:
			void FlushAll(bool cleanup);
			
		private:
			Spinlocker _locker;
			std::set<std::string> _tables;
			std::unordered_map<u64, db_object*> _objects;

		private:
			bool CheckAndCreateTable(EasydbInternal* easydb, std::string table);
		    bool CreateTable(EasydbInternal* easydb, std::string table);
		    u64  InsertObjectToTable(EasydbInternal* easydb, std::string table, u64 id, const ByteBuffer* buffer);
		    bool GetObjectFromTable(EasydbInternal* easydb, std::string table, u64 id, ByteBuffer* buffer);
		    bool DeleteObjectFromTable(EasydbInternal* easydb, std::string table, u64 id);
		    bool FlushObjectToTable(EasydbInternal* easydb, std::string table, db_object* object);			
		    bool FlushBufferToTable(EasydbInternal* easydb, std::string table, u64 id, const ByteBuffer* buffer);
	};
}

#define sObjectManager tnode::Singleton<tnode::ObjectManager>::getInstance()

#endif

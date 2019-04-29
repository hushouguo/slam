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
		ByteBuffer data;
		db_object(EasydbInternal* _easydb, std::string _table, u64 _id, bool _dirty)
			: easydb(_easydb)
			, table(_table)
			, id(_id)
			, dirty(_dirty)
		{}
	};
	
	class ObjectManager {
		public:
			u64 createObject(EasydbInternal* easydb, std::string table, u64 id, Message*);
			ByteBuffer* retrieveObject(EasydbInternal* easydb, std::string table, u64 id);
			bool deleteObject(EasydbInternal* easydb, std::string table, u64 id);
			bool updateObject(EasydbInternal* easydb, std::string table, u64 id, Message*);
			bool flushObject(EasydbInternal* easydb, std::string table, u64 id);

		public:
			void FlushAll(EasydbInternal* easydb, bool cleanup);
			
		private:
			Spinlocker _locker;
			std::unordered_map<u64, db_object*> _objects;

		private:
		    bool InsertObjectToTable(EasydbInternal* easydb, std::string table, db_object* object);
		    bool GetObjectFromTable(EasydbInternal* easydb, std::string table, db_object* object);
		    bool DeleteObjectFromTable(EasydbInternal* easydb, std::string table, u64 id);
		    bool FlushObjectToTable(EasydbInternal* easydb, std::string table, db_object* object);

		private:
			bool InsertObjectToCache(db_object* object);
			db_object* GetObjectFromCache(u64 id);
			bool DeleteObjectFromCache(u64 id);
	};
}

#define sObjectManager tnode::Singleton<tnode::ObjectManager>::getInstance()

#endif

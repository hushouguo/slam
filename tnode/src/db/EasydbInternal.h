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
			void stop() override;
			bool connectServer(std::string host, std::string user, std::string passwd, int port) override;

		public:
			bool createDatabase(std::string) override;
			bool selectDatabase(std::string) override;
			bool deleteDatabase(std::string) override;
			bool findDatabase(std::string) override;
			
		public:
			bool createObject(std::string table, Object* object) override;
			Object* retrieveObject(std::string table, uint64_t id) override;

		private:
			bool _isstop = true;
			std::string _database;
			MySQL* _dbhandler = nullptr;
		    std::unordered_map<std::string, std::unordered_map<u64, Object*>> _objects;
            			
		private:
		    bool createTable(std::string table);
		    bool insertObject(std::string table, Object* object);
		    bool retrieveObject(std::string table, Object* object);
	};
}

#endif

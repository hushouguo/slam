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
			bool deleteDatabase(std::string) override;
			bool findDatabase(std::string) override;
			
		public:
			u64 createObject(std::string table, u64 id, google::protobuf::Message*) override;
			google::protobuf::Message* retrieveObject(std::string table, u64 id) override;
			bool deleteObject(std::string table, u64 id) override;
			bool updateObject(std::string table, u64 id, google::protobuf::Message*) override;

		public:
			luaT_message_parser* tableParser() override { return this->_tableParser; }

		private:
			luaT_message_parser* _tableParser = nullptr;
		
		private:
			bool _isstop = true;
			void stop();
			std::string _database;
			MySQL* _dbhandler = nullptr;
		    std::unordered_map<std::string, std::unordered_map<u64, google::protobuf::Message*>> _objects;
            			
		private:
		    bool createTable(std::string table);
		    u64 addObject(std::string table, u64 id, const ByteBuffer* buffer);
		    bool getObject(std::string table, u64 id, ByteBuffer* buffer);
	};
}

#endif

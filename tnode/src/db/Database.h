/*
 * \file: Database.h
 * \brief: Created by hushouguo at 17:38:56 Apr 11 2019
 */
 
#ifndef __DATABASE_H__
#define __DATABASE_H__

BEGIN_NAMESPACE_TNODE {
	class Database {
		public:
			virtual ~Database() = 0;

		public:
			virtual bool connectDatabase(const char* host, const char* user, const char* passwd, const char* db, u16 port) = 0;
			virtual void closeDatabase() = 0;

		public:
			virtual TableManager* tableManager() = 0;
			virtual void flush() = 0;
	};
}

#endif

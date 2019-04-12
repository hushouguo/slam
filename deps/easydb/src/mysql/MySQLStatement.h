/*
 * \file: MySQLStatement.h
 * \brief: Created by hushouguo at 17:24:19 Apr 11 2019
 */
 
#ifndef __MYSQLSTATEMENT_H__
#define __MYSQLSTATEMENT_H__

namespace db {	
	class MySQL;
	class MySQLStatement {
		public:
			MySQLStatement(MySQL* owner);
			~MySQLStatement();

		public:
			bool prepare(std::string s);
			bool bindParam(MYSQL_BIND* b);
			bool exec();
			int  rowCount();
			bool bindResult(MYSQL_BIND* b);
			bool fetch();
			bool fetchColumn(MYSQL_BIND* b, unsigned int column, unsigned long offset = 0);
			bool freeResult();
			unsigned int lastError();

		private:
			MySQL* _owner = nullptr;
			MYSQL_STMT* _stmt = nullptr;
	};
}

#endif

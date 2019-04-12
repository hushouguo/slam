/*
 * \file: MySQLResult.h
 * \brief: Created by hushouguo at 17:26:26 Apr 11 2019
 */
 
#ifndef __MYSQLRESULT_H__
#define __MYSQLRESULT_H__

namespace db {
	class MySQLResult {
		public:
			MySQLResult(MYSQL_RES* res);
			~MySQLResult();

		public:
			void free();
			MYSQL_ROW fetchRow();
			u64 rowNumber();
			u32 fieldNumber();
			MYSQL_FIELD* fetchField();

		private:
			MYSQL_RES* _res = nullptr;
	};
}

#endif

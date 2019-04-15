/*
 * \file: MySQLResult.cpp
 * \brief: Created by hushouguo at 17:26:45 Apr 11 2019
 */

#include "tnode.h"

BEGIN_NAMESPACE_TNODE {
	MySQLResult::MySQLResult(MYSQL_RES* res) : _res(res) {
	}

	MySQLResult::~MySQLResult() {
		this->free();
	}

	void MySQLResult::free() {
		mysql_free_result(this->_res);
	}

	MYSQL_ROW MySQLResult::fetchRow() {
		return mysql_fetch_row(this->_res);
	}

	u64 MySQLResult::rowNumber() {
		return mysql_num_rows(this->_res);
	}

	u32 MySQLResult::fieldNumber() {
		return mysql_num_fields(this->_res);
	}
	
	MYSQL_FIELD* MySQLResult::fetchField() {
		return mysql_fetch_fields(this->_res);
	}
}


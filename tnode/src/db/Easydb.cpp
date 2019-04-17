/*
 * \file: Easydb.cpp
 * \brief: Created by hushouguo at 14:19:00 Apr 15 2019
 */

#include "tnode.h"
#include "tools/Tools.h"
#include "message/MessageParser.h"
#include "mysql/MySQLResult.h"
#include "mysql/MySQLStatement.h"
#include "mysql/MySQL.h"
#include "lua/luaT_message_parser.h"
#include "Easydb.h"
#include "EasydbInternal.h"

BEGIN_NAMESPACE_TNODE {
	Easydb* Easydb::createInstance() {
		return new EasydbInternal();
	}
}


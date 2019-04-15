/*
 * \file: test_luaT.cpp
 * \brief: Created by hushouguo at 11:16:56 Apr 15 2019
 */

#include "Easylog.h"
#include "luaT.h"

using namespace luaT;

void test_luaT() {
	lua_State* L = luaT_newstate();
	luaT_showversion(L);
	luaT_message_parser* parser = luaT_message_parser_creator::create();
	bool rc = parser->loadmsg("echo.proto");
	assert(rc);
	rc = parser->regmsg(1, "protocol.EchoRequest");
	assert(rc);
	delete parser;
	luaT_close(L);
}

int main() {
	logger::Easylog::syslog()->set_autosplit_day(true);
	logger::Easylog::syslog()->set_destination(".logs");
	logger::Easylog::syslog()->set_tofile(logger::GLOBAL, "tests.luaT");

	test_luaT();

	logger::Easylog::syslog()->stop();
	return 0;
}


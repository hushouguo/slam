/*
 * \file: test_easydb.cpp
 * \brief: Created by hushouguo at 17:08:42 Apr 12 2019
 */

#include "Easylog.h"
#include "Easydb.h"

using namespace db;

void test_easydb() {
	Easydb* easydb = Easydb::createInstance();
	bool rc = easydb->connectServer("127.0.0.1", "root", "", 3306);
	CHECK_RETURN(rc, void(0), "connectServer error");
	rc = easydb->createDatabase("easydb");
	CHECK_RETURN(rc, void(0), "createDatabase error");
	rc = easydb->selectDatabase("easydb");
	CHECK_RETURN(rc, void(0), "selectDatabase error");
	Entity* entity = new Entity(100);
	entity->SetValue("name", "hushouguo+");
	entity->SetValue("age", 0x1affffffff);
	entity->Dump();
	rc = easydb->serialize("player", entity);

	delete easydb;
	Debug << "easydb test OK";
}

int main() {
	logger::Easylog::syslog()->set_autosplit_day(true);
	logger::Easylog::syslog()->set_destination(".logs");
	logger::Easylog::syslog()->set_tofile(logger::GLOBAL, "tests.easydb");

	test_easydb();

	logger::Easylog::syslog()->stop();
	return 0;
}



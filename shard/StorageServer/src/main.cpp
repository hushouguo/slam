/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 15:10:52 Mar 25 2019
 */

#include "common.h"
#include "CentralTask.h"
#include "CentralTaskManager.h"
#include "SceneTask.h"
#include "SceneTaskManager.h"
#include "StorageService.h"
#include "MainProcess.h"
#include "StorageEntity.h"
#include "StorageEntityManager.h"
#include "MessageStatement.h"
#include "StorageHandler.h"
#include "StorageHandlerManager.h"
#include "StorageProcess.h"
#include "StorageProcessManager.h"

using namespace slam;
int main(int argc, char* argv[]) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	//
	// parser command line arguments
	//
	if (!sConfig.init(argc, argv)) { return false; }

	//
	// limit
	//
	if (!VerifyLimits()) { return false; }

	//
	// install signal handler
	//
	InstallSignalHandler(0, [](int sig) {
		sMainProcess.wakeup();
	});

	//
	// config Easylog
	//
	Easylog::syslog()->set_level((EasylogSeverityLevel) sConfig.get("log.level", GLOBAL));
	Easylog::syslog()->set_autosplit_day(sConfig.get("log.autosplit_day", true));
	Easylog::syslog()->set_autosplit_hour(sConfig.get("log.autosplit_hour", false));
	Easylog::syslog()->set_destination(sConfig.get("log.dir", ".logs"));
	Easylog::syslog()->set_tofile(GLOBAL, getProgramName());
	Easylog::syslog()->set_tostdout(GLOBAL, sConfig.runasdaemon ? false : true);

	DumpLibraryVersion();

	sThreadPool.init(sConfig.get("Service.threads", sConfig.threads));

	if (true) {
		auto createEntity = [](StorageHandlerManager& m) {
			Entity entity;
			//entity.set_id(100);
			entity.set_gold(100);
			entity.set_diamond(200);
			entity.set_bag("this is a bagaaa背包sdfsd");
			entity.set_level(123);
			entity.mutable_friendlist()->set_id(101);
			entity.mutable_friendlist()->set_name("hushouguo");
			for (int i = 10;i<20;++i) {
				entity.add_enemy(i);
			}
			for (char c = 'a'; c <= 'z'; ++c) {
				std::ostringstream o;
				o << c;
				entity.add_alias(o.str());
			}
			entity.set_mailbox("mailbox");
			u64 entityid = m.InsertEntityToTable(1, "user", &entity);
			Debug << "entityid: " << entityid;
			DumpMessage(&entity);
		};

		auto loadEntity = [](StorageHandlerManager& m, u64 entityid, Entity* entity) {
			bool rc = m.RetrieveEntityFromTable(1, "user", entityid, entity);
			if (rc) {
				DumpMessage(entity);
			}
			else {
				Error << "loadEntity error: " << entityid;
			}
		};

		auto updateEntity = [&loadEntity](StorageHandlerManager& m, u64 entityid, Entity* entity) {
			//entity->set_diamond(entity->diamond() + 1);
			bool rc = m.UpdateEntityToTable(1, "user", entityid, entity);
			if (rc) {
				//loadEntity(m, entityid);
			}
			else {
				Error << "updateEntity error: " << entityid;
			}
		};

		StorageHandlerManager m;
		CHECK_GOTO(m.init(), exit_failure, "StorageHandlerManager init failure");
		if (false) {
			createEntity(m);
		}
		else {
			Entity entity;
			loadEntity(m, 1, &entity);
			//entity.set_mailbox("this is new mailbox");
			entity.set_mailbox("");
			entity.set_gold(0);
			updateEntity(m, 1, &entity);
			Debug << "load entity: 1 again";
			loadEntity(m, 1, &entity);
		}
		goto exit_failure;		
	}
	
	CHECK_GOTO(sStorageProcessManager.init(
		sConfig.get("Service.threads", sConfig.threads)),
		exit_failure, "StorageProcessManager init failure");

	CHECK_GOTO(sStorageService.init(
		sConfig.get("Service.address", "0.0.0.0"), sConfig.get("Service.port", 12300u)),
		exit_failure, "StorageService init failure");
	
	sMainProcess.run();

exit_failure:
	sConfig.syshalt(0);
	sThreadPool.stop();
	sStorageProcessManager.stop();
	sStorageService.stop();
	Trace("shutdown system with terminate reason: %d", sConfig.terminate_reason);
	Easylog::syslog()->stop();
	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();
	return 0;	
}


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


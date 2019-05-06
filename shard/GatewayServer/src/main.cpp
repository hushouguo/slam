/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 15:10:52 Mar 25 2019
 */

#include "common.h"
#include "ClientTask.h"
#include "ClientTaskManager.h"
#include "CentralClient.h"
#include "SceneClient.h"
#include "SceneClientManager.h"
#include "GatewayPlayer.h"
#include "GatewayPlayerManager.h"
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
	InstallSignalHandler((1 << SIGINT)|(1 << SIGTERM)|(1 << SIGQUIT)|(1 << SIGHUP)|(1 << SIGUSR1)|(1 << SIGUSR2), [](int sig) {
		switch (sig) {
			case SIGINT: case SIGTERM: case SIGQUIT: 
				sConfig.halt = true; break;
			case SIGHUP: 
				sConfig.reload = true; break;
			case SIGUSR1:
			case SIGUSR2:
				//dump system runtime information
				tc_malloc_stats(); break;			
			default: sConfig.syshalt(sig); break;
		}
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

#if 0
	CHECK_GOTO(sCentralClient.init(
		sConfig.get("CentralServer.address", "127.0.0.1"), sConfig.get("CentralServer.port", 9000u)), 
		exit_failure, "CentralClient init failure");
#endif

	CHECK_GOTO(sClientTaskManager.init(
		sConfig.get("Service.address", "0.0.0.0"), sConfig.get("Service.port", 12306u)), 
		exit_failure, "ClientTaskManager init failure");
	
//	CHECK_GOTO(sSceneClientManager.init(), exit_failure, "SceneClientManager init failure");

	sMainProcess.run();

exit_failure:
	sConfig.syshalt(0);
	sClientTaskManager.stop();
	sCentralClient.stop();
	sSceneClientManager.stop();
	Trace("shutdown system with terminate reason: %d", sConfig.terminate_reason);
	Easylog::syslog()->stop();
	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();
	return 0;	
}


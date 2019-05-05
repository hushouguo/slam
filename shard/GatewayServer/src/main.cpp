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

void install_signal_handler() {
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
	act.sa_handler = [](int sig) {
		//fprintf(stderr, "receive signal: %d\n", sig);
		// Don't call Non reentrant function, just like malloc, free etc, i/o function also cannot call.
		//if (sig == SIGRTMIN) {		// SIGRTMIN: Wake up thread, nothing to do
		//	return;	// SIGRTMIN: #define SIGRTMIN        (__libc_current_sigrtmin ())
		//}
		switch (sig) {
			case SIGINT:
			case SIGTERM:
			case SIGQUIT: 
				sConfig.halt = true;
				sMainProcess.wakeup();
				break;	// Note: schedule halt
		
			case SIGHUP: 
				sConfig.reload = true; break; // NOTE: reload configure file

			case SIGUSR1:
			case SIGUSR2:
				//dump system runtime information
				//tc_malloc_stats();
				break;
			
			case SIGWINCH: break;	// the window size change, ignore
			//case SIGALRM: break;	// timer expire
			
			default: sConfig.syshalt(sig); break;
		}
	};

	sigaction(SIGINT, &act, nullptr);
	sigaction(SIGTERM, &act, nullptr);
	sigaction(SIGQUIT, &act, nullptr);
	sigaction(SIGHUP, &act, nullptr);
	sigaction(SIGUSR1, &act, nullptr);
	sigaction(SIGUSR2, &act, nullptr);
	//sigaction(SIGALRM, &act, nullptr);
	//sigaction(SIGRTMIN, &act, nullptr);
}

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
	install_signal_handler();

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


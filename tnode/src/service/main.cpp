/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 15:10:52 Mar 25 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "tools/Entry.h"
#include "tools/Tools.h"
#include "tools/Spinlocker.h"
#include "tools/LockfreeQueue.h"
#include "tools/LockfreeMap.h"
#include "tools/ThreadPool.h"
#include "tools/Runnable.h"
#include "tools/Registry.h"
#include "config/Config.h"
#include "message/ServiceMessage.h"
#include "message/MessageParser.h"
#include "time/Time.h"
#include "lua/luaT.h"
#include "lua/luaT_message_parser.h"
#include "db/Easydb.h"
#include "service/Service.h"
#include "service/ServiceManager.h"
#include "net/NetworkManager.h"

using namespace tnode;

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
				sConfig.halt = true; break;	// Note: schedule halt
		
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

bool verify_limits() {
	//
	// limit
	//
	size_t stack_size = sConfig.get("limit.stack_size", 0u);
	if (stack_size > 0) {
		setStackSizeLimit(stack_size);
	}

	size_t max_files = sConfig.get("limit.max_files", 0u);
	if (max_files > 0) {
		setOpenFilesLimit(max_files);
	}

	Trace("stack size: %u (limit.stack_size), max files: %u (limit.max_files)", getStackSizeLimit(), getOpenFilesLimit());

	//
	// verify lua version
	CHECK_RETURN(sizeof(lua_Integer) == 8, false, "require right version for lua");
	CHECK_RETURN(sizeof(lua_Number) == 8, false, "require right version for lua");

	return true;
}

void dump_library_version() {
	//
	// tnode
	//
#ifdef DEBUG		
	Trace("tnode: %d.%d.%d, threads: %d, run as %s, %s, debug", TNODE_VERSION_MAJOR, TNODE_VERSION_MINOR, TNODE_VERSION_PATCH, sConfig.threads, sConfig.runasdaemon ? "daemon" : "console", sConfig.guard ? "with guard" : "no guard");
#else		
	Trace("tnode: %d.%d.%d, threads: %d, run as %s, %s, release", TNODE_VERSION_MAJOR, TNODE_VERSION_MINOR, TNODE_VERSION_PATCH, sConfig.threads, sConfig.runasdaemon ? "daemon" : "console", sConfig.guard ? "with guard" : "no guard");
#endif

	//
	// Config information
	//
	if (sConfig.confile.empty()) {
		Alarm << "specify config file: Unspecified";
	}
	else {
		Trace << "specify config file: " << sConfig.confile;
	}
	sConfig.dump();

#if 0
	//
	// Easylog configure information
	//
	extern const char* tnode::level_string(EasylogSeverityLevel);
	Trace("Easylog:");
	Trace("    log.level: %s", level_string(Easylog::syslog()->level()));
	Trace("    log.autosplit_day: %s, log.autosplit_hour: %s", 
			Easylog::syslog()->autosplit_day() ? "yes" : "no", 
			Easylog::syslog()->autosplit_hour() ? "yes" : "no");
	Trace("    log.dir: %s", Easylog::syslog()->destination());
#endif

	
	//
	// output 3rd libraries
	//
	Trace("all 3rd libraries:");

#ifdef TC_VERSION_MAJOR		
	Trace("    tcmalloc: %d.%d%s", TC_VERSION_MAJOR, TC_VERSION_MINOR, TC_VERSION_PATCH);
#else
	Trace("    not link tcmalloc");
#endif

#ifdef LIBEVENT_VERSION
	Trace("    libevent: %s", LIBEVENT_VERSION);
#endif

#ifdef ZMQ_VERSION_MAJOR
	Trace("    libzmq: %d.%d.%d", ZMQ_VERSION_MAJOR, ZMQ_VERSION_MINOR, ZMQ_VERSION_PATCH);
#endif

#ifdef LUAJIT_VERSION
	Trace("    luaJIT: %s -- %s", LUAJIT_VERSION, LUAJIT_COPYRIGHT);
#endif

#ifdef GOOGLE_PROTOBUF_VERSION
	Trace("    protobuf: %d, library: %d", GOOGLE_PROTOBUF_VERSION, GOOGLE_PROTOBUF_MIN_LIBRARY_VERSION);
#endif

	Trace("    rapidxml: 1.13");

#ifdef MYSQL_SERVER_VERSION		
	Trace("    mysql: %s", MYSQL_SERVER_VERSION);
#endif

	Trace("    gcc version: %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);

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
	if (!verify_limits()) { return false; }

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

	dump_library_version();

	//
	// init thread pool
	sThreadPool.init(sConfig.threads);

	//
	// init Easynet
	sNetworkManager.init();

	//
	// init Service
	CHECK_GOTO(sServiceManager.init(sConfig.entryfile.c_str()), exit_failure, "ServiceManager init failure");

	//
	// delivery message to service
	// scheduling service
	// todo: monitor, control server by command line, reload config etc...
	while (!sConfig.halt) {
		sTime.now();
		sNetworkManager.run();
		sServiceManager.schedule();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

exit_failure:
	sConfig.syshalt(0);
	sServiceManager.stop();
	sNetworkManager.stop();
	sThreadPool.stop();
	Trace("shutdown system with terminate reason: %d", sConfig.terminate_reason);
	Easylog::syslog()->stop();
	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();
	return 0;	
}


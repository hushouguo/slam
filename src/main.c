/*
 * \file: main.c
 * \brief: Created by hushouguo at 15:30:03 Jul 26 2019
 */

#include "slam.h"

void slam_signal_handler(int sig) {
    switch (sig) {
        case SIGHUP:
        	slam_main()->reload_lua = true;
        	slam_main()->reload_dynamic_lib = true;
            break;
            
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
        	slam_main()->halt = true;
            break;
            
        case SIGUSR1:
       	case SIGUSR2:
            //dump system runtime information
            tc_malloc_stats();
            break;
            
        default: Debug("unhandled signal: %d", sig); break;
    };
}            

void slam_install_signal_handler() {
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	//act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
	act.sa_flags = 0;
	act.sa_handler = slam_signal_handler;
    sigaction(SIGHUP, &act, nullptr);	// reload script file & reload dynamic lib
    sigaction(SIGINT, &act, nullptr);	// exit
    sigaction(SIGTERM, &act, nullptr);	// exit
    sigaction(SIGQUIT, &act, nullptr);	// exit
    sigaction(SIGUSR1, &act, nullptr);	// reserved
    sigaction(SIGUSR2, &act, nullptr);	// reserved
}

int main(int argc, char** argv) {
	CHECK_RETURN(argc > 1, -1, "Usage: ./slam entryfile");
	slam_mallinfo();
	slam_install_signal_handler();
	if (!slam_runnable_load_protocol(slam_main()->runnable, "./protocol/libprotocol.so")) {
		return -1;
	}
	if (!slam_runnable_load_entryfile(slam_main()->runnable, argv[1])) {
		return -1;
	}
	while (!slam_main()->halt) {
		slam_runnable_run(slam_main()->runnable);
		if (slam_main()->reload_dynamic_lib) {
			Trace("reload dynamic lib");
			slam_runnable_load_protocol(slam_main()->runnable, "./protocol/libprotocol.so");
		}
	}
	return 0;
}


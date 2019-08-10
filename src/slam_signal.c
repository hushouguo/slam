/*
 * \file: slam_signal.c
 * \brief: Created by hushouguo at 14:27:05 Aug 07 2019
 */

#include "slam.h"

static void slam_signal_handler(int sig) {
    Debug("receive signal: %d", sig);
    switch (sig) {
        case SIGHUP:
        	__slam_main->reload_entryfile = true;
            break;
            
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
        	__slam_main->halt = true;
            break;
            
        case SIGUSR1:
       	case SIGUSR2:
            break;
            
        default: Alarm("unhandled signal: %d", sig); break;
    };
}            

void slam_install_signal_handler() {
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	//act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
	act.sa_flags = 0;
	act.sa_handler = slam_signal_handler;
    sigaction(SIGHUP, &act, nullptr);	// reload script file & reload protocol
    sigaction(SIGINT, &act, nullptr);	// exit
    sigaction(SIGTERM, &act, nullptr);	// exit
    sigaction(SIGQUIT, &act, nullptr);	// exit
    sigaction(SIGUSR1, &act, nullptr);	// reserved
    sigaction(SIGUSR2, &act, nullptr);	// reserved
}


/*
 * \file: slam_main.c
 * \brief: Created by hushouguo at 11:17:03 Jul 30 2019
 */

#include "slam.h"

#define SLAM_DEF_LIMIT_STACK_SIZE		16*MB	// limits, linux default stack size: 8M (soft), 4G (hard)
#define SLAM_DEF_LIMIT_OPEN_FILES		8192	// limits, linux default open files: 1024 (soft), 4096 (hard)
#define SLAM_DEF_CONN_MAX_SILENT		10		// seconds
#define SLAM_DEF_CONN_CHECK_INTERVAL	10		// seconds
#define SLAM_DEF_CONN_MAX_MESSAGES		300		// the number of messages
#define SLAM_DEF_POLL_EVENT_NUMBER      1024
#define SLAM_DEF_MAX_PACKAGE_SIZE       4*MB	// bytes
#define SLAM_DEF_MESSAGE_QUEUE_SIZE     10000
#define SLAM_DEF_SEND_QUEUE_SIZE        1000
#define SLAM_DEF_LOGFILE                "slam.log"

slam_main_t* __slam_main = nullptr;


void Usage() {
	fprintf(stderr, "Usage: slam [OPTIONS]\n");
	fprintf(stderr, "    OPTIONS:\n");
	fprintf(stderr, "      -d:          run as daemon, (%s)\n", __slam_main->runasdaemon ? "true" : "false");
	fprintf(stderr, "      -w number    worker number, (%d)\n", __slam_main->worker_number);
	fprintf(stderr, "      -e filename  lua entryfile, (%s)\n", __slam_main->entryfile ? __slam_main->entryfile : "nullptr");
	fprintf(stderr, "      -l filename  logfile, (%s)\n", __slam_main->logfile);
}

bool slam_main_parse_args(int argc, char** argv) {
	int c;
	/*--argc, argv++;*/
	while ((c = getopt(argc, argv, "dDhHe:w:l:")) != -1) {
		switch (c) {
			case 'd': case 'D': __slam_main->runasdaemon = true; break;
			case 'h': case 'H': default: Usage(); return false;
			case 'e': __slam_main->entryfile = strdup(optarg); break;
			case 'w': __slam_main->worker_number = atoi(optarg); break;
			case 'l': __slam_main->logfile = strdup(optarg); break;
		}
	}
	return true;
}

void slam_main_dump() {
	fprintf(stderr, "slam version: %d.%d.%d, worker: %d, entryfile: %s, logfile: %s, %s\n", 
			SLAM_VERSION_MAJOR, SLAM_VERSION_MINOR, SLAM_VERSION_PATCH,
			__slam_main->worker_number,
			__slam_main->entryfile ? __slam_main->entryfile : "nullptr",
			__slam_main->logfile,
			__slam_main->runasdaemon ? "run as daemon" : "run as console"
			);
}

bool slam_main_spawn_child_process() {
	if (!fork()) {
		__slam_main->runasmaster = false;
		slam_set_process_title(__slam_main->argc, __slam_main->argv, " worker");
	}
	return true;
}

bool slam_main_init_child_process() {
	if (!__slam_main->runasmaster) { // child process continue initialization
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, __slam_main->fd_io_sockets) != 0) {
            return false;
        }        	    
        __slam_main->mq = slam_message_queue_new(__slam_main->receive_queue_size);
		__slam_main->runnable = slam_runnable_new();
		if (!__slam_main->runnable) {
			return false;
		}
		__slam_main->io = slam_io_thread_new(__slam_main->mq);
		if (!__slam_main->io) {
			return false;
		}
		if (!slam_runnable_load_entryfile(__slam_main->runnable, __slam_main->entryfile)) {
			return false;
		}
	}
	return true;
}

bool slam_main_init(int argc, char* argv[]) {
	setlocale(LC_ALL, "");

	if (!__slam_main) {
		__slam_main = (slam_main_t *) slam_malloc(sizeof(slam_main_t));
		
		__slam_main->halt = false;
		__slam_main->runasdaemon = false;
		__slam_main->runasmaster = true;
	
		__slam_main->argc = argc;
		__slam_main->argv = argv;

		__slam_main->reload_entryfile = false;
		__slam_main->entryfile = nullptr;
		__slam_main->logfile = strdup(SLAM_DEF_LOGFILE);
		
		__slam_main->limit_stack_size = SLAM_DEF_LIMIT_STACK_SIZE;
		__slam_main->limit_max_files = SLAM_DEF_LIMIT_OPEN_FILES;

		__slam_main->worker_number = slam_cpus();

		__slam_main->conn_max_silent = SLAM_DEF_CONN_MAX_SILENT;
		__slam_main->conn_check_interval = SLAM_DEF_CONN_CHECK_INTERVAL;
		__slam_main->conn_max_messages = SLAM_DEF_CONN_MAX_MESSAGES;

        __slam_main->poll_event_number = SLAM_DEF_POLL_EVENT_NUMBER;
        __slam_main->max_package_size = SLAM_DEF_MAX_PACKAGE_SIZE;
        __slam_main->receive_queue_size = SLAM_DEF_MESSAGE_QUEUE_SIZE;
        __slam_main->socket_send_queue_size = SLAM_DEF_SEND_QUEUE_SIZE;

        int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, __slam_main->fd_worker_sockets);
        CHECK_RETURN(rc == 0, false, "socketpair error: %d, %s", errno, errstring(errno));
        
        __slam_main->log = slam_log_new(__slam_main->logfile);
		__slam_main->mq = nullptr;
		__slam_main->runnable = nullptr;
		__slam_main->io = nullptr;
	}

	if (!slam_main_parse_args(argc, argv)) {
		return false;
	}

    CHECK_RETURN(__slam_main->log, false, "logfile: %s error", __slam_main->logfile);
	CHECK_RETURN(__slam_main->worker_number > 0, false, "the number of worker is 0");
	CHECK_RETURN(__slam_main->entryfile, false, "entryfile is nullptr");
	CHECK_RETURN(slam_file_exist(__slam_main->entryfile), false, "entryfile: %s inexist", __slam_main->entryfile);
	CHECK_RETURN(slam_file_readable(__slam_main->entryfile), false, "entryfile: %s unreadable", __slam_main->entryfile);

	if (__slam_main->runasdaemon) {
		daemon(1, 0); /* 1: nochdir, 1: noclose */
		close(1);
		close(2);
		dup2(__slam_main->log->fd, 1);
		dup2(__slam_main->log->fd, 2);
	}

	slam_main_dump();
	slam_install_signal_handler();

	do {
		uint32_t i = 0;
		for (; i < __slam_main->worker_number; ++i) {
		    slam_main_spawn_child_process();
		    if (!__slam_main->runasmaster) {
    			if (!slam_main_init_child_process()) {
    				slam_main_exit(EXIT_FAILURE);
    			}
    			break; // child process exit loop
			}
		}
	} while (false);	

	return true;
}

void slam_main_exit(int status) {
	if (__slam_main) {
		__slam_main->halt = true;

		slam_free(__slam_main->entryfile);
		slam_free(__slam_main->logfile);

        slam_close(__slam_main->fd_worker_sockets[SLAM_FD_WORKER]);
        slam_close(__slam_main->fd_worker_sockets[SLAM_FD_MASTER]);
		
        slam_log_delete(__slam_main->log);
        
		if (!__slam_main->runasmaster) {            
		    if (__slam_main->runnable) {
    			slam_runnable_delete(__slam_main->runnable);
			}
			if (__slam_main->io) {
    			slam_io_thread_delete(__slam_main->io);
			}
			if (__slam_main->mq) {
    			slam_message_queue_delete(__slam_main->mq);
			}			
            slam_close(__slam_main->fd_io_sockets[SLAM_FD_IO]);
            slam_close(__slam_main->fd_io_sockets[SLAM_FD_WORKER]);
		}
	
		log_trace("process: %d, %s, exit: %d", getpid(), (__slam_main->runasmaster ? "master" : "worker"), status);

		slam_free(__slam_main);
	}
	exit(status);
}

void slam_main_run_master() {
	int status = 0;
	pid_t pid = wait(&status);
	(void)(pid);
	if (__slam_main->halt) {
		return;
	}
	if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE) {
		exit(EXIT_FAILURE);
	}
	else {
	    slam_main_spawn_child_process();
        if (!__slam_main->runasmaster) {
            if (!slam_main_init_child_process()) {
                log_error("child process: %d, exit status: %d(%s), reload init error", 
                        pid, WEXITSTATUS(status), WIFEXITED(status) ? "normal" : "abnormal");
                slam_main_exit(EXIT_FAILURE);
            }
        }
	}
}

void slam_main_run_worker() {
	slam_runnable_run(__slam_main->runnable);
}

void slam_main_run() {
	while (!__slam_main->halt) {
		if (__slam_main->runasmaster) {
			slam_main_run_master();
		}
		else {
			slam_main_run_worker();
		}
	}
}

int main(int argc, char** argv) {
	if (!slam_main_init(argc, argv)) {
	    return EXIT_FAILURE;
	}
	slam_main_run();
	slam_main_exit(SLAM_OK);
	log_trace("Bye!");
	return 0;
}

__attribute__((constructor)) static void __slam_main_init() {
}

__attribute__((destructor)) static void __slam_main_destroy() {
}


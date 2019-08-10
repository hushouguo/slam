/*
 * \file: slam.h
 * \brief: Created by hushouguo at 10:58:42 Jul 25 2019
 */
 
#ifndef __SLAM_H__
#define __SLAM_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h> 
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include <linux/limits.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"

// tcmalloc-2.6.1
#include <gperftools/tcmalloc.h>


#if !defined(__plusplus)
#define nullptr NULL
#endif

#if !defined(__plusplus)
typedef int 	bool;
#define true	1	/* rc == rc is 1 */
#define false	0	/* rc != rc is 0 */
#endif

typedef unsigned char byte_t;
typedef int msgid_t;
typedef unsigned int msglen_t;
typedef uint64_t entityid_t;

#ifdef PLATFORM_WINDOWS
#define SOCKET						intptr_t
#else
#define SOCKET						int
#endif

#define INVALID_SOCKET			    -1
#define SOCKET_CONNECT_TIMEOUT		5 /* seconds */


#ifdef TC_VERSION_STRING
#define slam_malloc(size) tc_malloc(size)
#define slam_free(var)	do { if (var) { tc_free((void*) (var)); (var) = nullptr; } } while(0)
#else
#define slam_malloc(size) malloc(size)
#define slam_free(var)	do { if (var) { free((void*) (var)); (var) = nullptr; } } while(0)
#endif
#define slam_close(fd)	do { if (fd >= 0) { close(fd); fd = -1; } } while(0)
#define slam_shutdown(fd, how)	do { if (fd >= 0) { shutdown(fd, how); fd = -1; } } while(0)

#define SLAM_OK          0
#define SLAM_ERROR      -1
#define SLAM_PANIC		-2

#if !defined(PATH_MAX)
#define PATH_MAX						MAX_PATH
#endif

#if !defined(TEMP_FAILURE_RETRY)
#define TEMP_FAILURE_RETRY(EXEC)		EXEC
#endif

#if !defined(MSG_DONTWAIT)
#define MSG_DONTWAIT					0
#endif

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL					0
#endif

#if !defined(offsetof)
#define offsetof(STRUCTURE,FIELD)		((size_t)((char*)&((STRUCTURE*)0)->FIELD))
#endif

#if defined(_MSC_VER)
    /*  Microsoft */
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    /*  GCC */
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    /*  do nothing and hope for the best? */
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#define	KB								1024U
#define MB								1048576U
#define GB								1073741824U
#define TB								1099511627776ULL

#define MINUTE							60U
#define HOUR							3600U
#define DAY								86400U

#include "MessageBuilder/MessageBuilder.h"

#include "slam_log.h"

#include "io/slam_atomic.h"
#include "io/slam_spinlock.h"
#include "io/slam_thread_cond.h"
#include "io/slam_message.h"
#include "io/slam_message_queue.h"
#include "io/slam_socket.h"
#include "io/slam_poll.h"
#include "io/slam_io_thread.h"

#include "slam_byte_buffer.h"
#include "slam_utils.h"
#include "slam_lua_value.h"
#include "slam_lua.h"
#include "slam_lua_func.h"
#include "slam_timer_list.h"
#include "slam_protocol.h"
#include "slam_runnable.h"
#include "slam_lua_event.h"
#include "slam_main.h"
#include "slam_signal.h"

#endif


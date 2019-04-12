/*
 * \file: Database.h
 * \brief: Created by hushouguo at 13:28:13 Apr 12 2019
 */
 
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include <map>
#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <exception>

using u8 	=	uint8_t;
using u16 	=	uint16_t;
using u32	=	uint32_t;
using u64	=	uint64_t;
using s8	=	int8_t;
using s16	=	int16_t;
using s32	=	int32_t;
using s64	=	int64_t;
using Byte	=	unsigned char;

#define SafeFree(P)			do { if(P) { ::free((void*)P); (P) = nullptr; } } while(0)
#define SafeDelete(P)		do { if(P) { delete (P); (P) = nullptr; } } while(0)
#define SafeClose(S)		do { if(S > 0) { ::close(S); S = -1; } } while(0)

#if EASYDB_ENABLE_DEBUG
#define Debug(MESSAGE, ...)	fprintf(stdout, "Easydb-Debug:" MESSAGE "\n", ##__VA_ARGS__)
#else
#define Debug(MESSAGE, ...)
#endif
#define Alarm(MESSAGE, ...)	fprintf(stderr, "Easydb-Alarm:" MESSAGE "\n", ##__VA_ARGS__)
#define Error(MESSAGE, ...)	fprintf(stderr, "Easydb-Error:" MESSAGE "\n", ##__VA_ARGS__)

#define CHECK_RETURN(RC, RESULT, MESSAGE, ...)	\
		do {\
			if (!(RC)) {\
				Error(MESSAGE, ##__VA_ARGS__);\
				return RESULT;\
			}\
		} while(false)
	
#define CHECK_GOTO(RC, SYMBOL, MESSAGE, ...)	\
		do {\
			if (!(RC)) {\
				Error(MESSAGE, ##__VA_ARGS__);\
				goto SYMBOL;\
			}\
		} while(false)
	
#define CHECK_BREAK(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Error(MESSAGE, ##__VA_ARGS__);\
			break;\
		}
	
#define CHECK_CONTINUE(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Error(MESSAGE, ##__VA_ARGS__);\
			continue;\
		}
	
#define CHECK_DEBUG(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Debug(MESSAGE, ##__VA_ARGS__);\
		}
			
#define CHECK_ALARM(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Alarm(MESSAGE, ##__VA_ARGS__);\
		}

#define CHECK_ERROR(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Error(MESSAGE, ##__VA_ARGS__);\
		}


#include "mysql/MySQLResult.h"
#include "mysql/MySQLStatement.h"
#include "mysql/MySQL.h"
#include "Easydb.h"
#include "EasydbInternal.h"

#endif

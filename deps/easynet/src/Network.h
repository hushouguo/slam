/*
 * \file: Network.h
 * \brief: Created by hushouguo at 17:25:35 Apr 09 2019
 */
 
#ifndef __NETWORK_H__
#define __NETWORK_H__

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
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

#include <map>
#include <string>
#include <list>
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

#include "ByteBuffer.h"
#include "Spinlocker.h"
#include "Helper.h"
#include "Easynet.h"
#include "Socket.h"
#include "SocketClient.h"
#include "SocketServer.h"
#include "Poll.h"
#include "EasynetInternal.h"

#if EASYNET_ENABLE_DEBUG
#define Trace(MESSAGE, ...)	fprintf(stdout, MESSAGE, ##__VA_ARGS__)
#else
#define Trace(MESSAGE, ...)
#endif

#endif

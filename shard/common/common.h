/*
 * \file: common.h
 * \brief: Created by hushouguo at 15:15:18 Apr 10 2019
 */
 
#ifndef __COMMON_H__
#define __COMMON_H__

#if defined(__plusplus)
#if __cplusplus <= 199711L
#error "REQUIRE C++ 11 SUPPORT"
#endif
#endif

#if !defined(X64)
#error "ONLY SUPPORT 64bit SYSTEM"
#endif

#if !defined(LINUX)
#error "ONLY SUPPORT LINUX PLATFORM"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stddef.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <linux/limits.h>
#include <pwd.h>
#include <execinfo.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <getopt.h>
#include <wchar.h>
#include <locale.h>
#include <libgen.h>

#include <cassert>
#include <cerrno>
#include <istream>
#include <random>
#include <iostream>
#include <iomanip>
#include <exception>
#include <fstream>  
#include <sstream>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <locale>
#include <limits>
#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>
#include <type_traits>
#include <chrono>
#include <algorithm>
#include <utility>
#include <vector>
#include <queue>
#include <array>
#include <list>
#include <string>
#include <cstring>
#include <csignal>
#include <map>
#include <stack>
#include <unordered_map>
#if __GNUC__ >= 5
#include <codecvt>
#endif

#include <mysql/mysql.h>
#include <mysql/errmsg.h>

#include <event.h>
#include <evhttp.h>

#include <curl/curl.h>

#include "Easylog.h"
#include "Easynet.h"

// tcmalloc-2.6.1
#include "gperftools/tcmalloc.h"

// rapidxml-1.13
#include "rapidxml.hpp"  
#include "rapidxml_utils.hpp"//rapidxml::file  
#include "rapidxml_print.hpp"//rapidxml::print 

// rapidjson-1.1.0
#include "document.h"

//luajit-2.0.5
#include "lua.hpp"

// google protobuf-3.6.1
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/message.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

using u8 	=	uint8_t;
using u16 	=	uint16_t;
using u32	=	uint32_t;
using u64	=	uint64_t;
using s8	=	int8_t;
using s16	=	int16_t;
using s32	=	int32_t;
using s64	=	int64_t;
using Byte	=	unsigned char;

#ifndef PATH_MAX
#define PATH_MAX						MAX_PATH
#endif
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(EXEC)		EXEC
#endif
#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT					0
#endif
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL					0
#endif

#ifndef offsetof
#define offsetof(STRUCTURE,FIELD)		((size_t)((char*)&((STRUCTURE*)0)->FIELD))
#endif

#if defined(_MSC_VER)
    //  Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
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

#define SafeFree(P)						do { if(P) { ::free((void*)P); (P) = nullptr; } } while(0)
#define SafeDelete(P)					do { if(P) { delete (P); (P) = nullptr; } } while(0)
#define SafeClose(S)					do { if(S > 0) { ::close(S); S = -1; } } while(0)

#define BEGIN_NAMESPACE_SLAM			namespace slam

using namespace net;
using namespace logger;

#include "tools/Singleton.h"
#include "tools/Callback.h"
#include "tools/Constructor.h"
#include "tools/Noncopyable.h"
#include "tools/base64.h"
#include "tools/ByteBuffer.h"
#include "tools/Entry.h"
#include "tools/Manager.h"
#include "tools/Registry.h"
#include "tools/Spinlocker.h"
#include "tools/LockfreeMap.h"
#include "tools/LockfreeQueue.h"
#include "tools/ThreadPool.h"
#include "tools/Runnable.h"
#include "tools/HttpParser.h"
#include "tools/Tools.h"
#include "xml/XmlParser.h"
#include "json/json_parser.h"
#include "csv/CsvReader.h"
#include "csv/CsvParser.h"
#include "config/Config.h"
#include "time/Time.h"
#include "time/Timer.h"
#include "message/CommonMessage.h"
#include "entity.pb.h"
#include "server.pb.h"

using namespace protocol;

#endif

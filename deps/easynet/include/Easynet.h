/*
 * \file: Easynet.h
 * \brief: Created by hushouguo at 10:29:59 Jul 06 2018
 */
 
#ifndef __EASYNET_H__
#define __EASYNET_H__

#define EASYNET_VERSION_MAJOR		0
#define EASYNET_VERSION_MINOR		1
#define EASYNET_VERSION_PATCH		0

#if defined(__plusplus)
#if __cplusplus <= 199711L
#error "REQUIRE C++ 11 SUPPORT"
#endif
#endif

#ifdef PLATFORM_WINDOWS
#define SOCKET						intptr_t
#else
#define SOCKET						int
#endif

#define EASYNET_ILLEGAL_SOCKET				-1
#define EASYNET_ENABLE_DEBUG				0
#define EASYNET_REUSE_ADDRESS				1
#define EASYNET_REUSE_PORT					0
#define EASYNET_RETURN_ACTIVE_MESSAGE		0

namespace net {	
	class Easynet {
		public:
			virtual ~Easynet() = 0;

		public:
			//
			// address, port
			virtual SOCKET createServer(const char*, int) = 0;
			//
			// address, port, seconds(connect timeout)
			virtual SOCKET createClient(const char*, int, int seconds) = 0;
			//
			// msg MUST be allocated by `allocateMessage` function
			virtual bool sendMessage(SOCKET s, const void* msg) = 0;
			//
			// return nullptr when no more message
			virtual const void* receiveMessage(SOCKET*) = 0;
			//
			// close socket
			virtual void closeSocket(SOCKET) = 0;
			//
			// `socket` is exist and active
			virtual bool isActive(SOCKET) = 0;
			//
			// return number of receiveQueue
			virtual size_t getQueueSize() = 0;
			//
			// return current total active connections
			virtual size_t totalConnections() = 0;
			//
			// state: true: establish connection, false: lost connection
			virtual SOCKET getSocketState(bool& state) = 0;

		public:
			//
			// allocate new message for special payload_len
			virtual const void* allocateMessage(size_t payload_len) = 0;
			//
			// release message
			virtual void releaseMessage(const void* msg) = 0;
			//
			// copy content to message data
			virtual void setMessageContent(const void* msg, const void* data, size_t len) = 0;
			//
			// return message data
			virtual const void* getMessageContent(const void* msg, size_t* len) = 0;
			//
			// return socket of message
			virtual SOCKET getMessageSocket(const void* msg) = 0;
		
		public:
			//
			// spliter: -1: error occupy, 0: incomplete package, > 0: len of package
			// messenger: when messages arrives, messenger will be called
			static Easynet* createInstance(std::function<int(const void*, size_t)> spliter, std::function<void()> messenger);

		public:
			virtual void stop() = 0;
	};
}

#endif

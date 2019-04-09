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
#define SOCKET							intptr_t
#else
#define SOCKET							int
#endif

namespace net {
	struct SocketMessage {
		SOCKET s;
		size_t payload_len;
		char payload[0];
	};
	SocketMessage* allocateSocketMessage(size_t payload_len);
	void releaseSocketMessage(const SocketMessage*);

	class Easynet {
		public:
			virtual ~Easynet() = 0;

		public:
			//
			// address, port, maxconn
			virtual SOCKET createServer(const char*, int, size_t) = 0;
			//
			// address, port, timeout(seconds), 
			virtual SOCKET createClient(const char*, int, size_t) = 0;
			//
			// `msg` MUST be allocated by allocateSocketMessage function
			virtual bool sendMessage(const SocketMessage* msg) = 0;
			//
			// return nullptr when no more Message
			virtual const SocketMessage* getMessage() = 0;
			//
			// close Socket right now
			virtual void closeSocket(SOCKET) = 0;
			//
			// `s` exist and active
			virtual bool isActive(SOCKET) = 0;

		public:
			//
			// spliter: -1: error occupy, 0: incomplete package, > 0: len of package
			static Easynet* createInstance(std::function<int(const void*, size_t)> spliter);

		public:
			virtual void stop() = 0;
	};
}

#endif

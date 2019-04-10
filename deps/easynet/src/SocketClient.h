/*
 * \file: SocketClient.h
 * \brief: Created by hushouguo at 01:56:02 Aug 09 2018
 */
 
#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

namespace net {
	class SocketClient : public Socket {
		public:
			virtual ~SocketClient() = 0;

		public:
			virtual bool connect(const char* address, int port) = 0;
	};

	class EasynetInternal;
	struct SocketClientCreator {
		static SocketClient* create(EasynetInternal* easynet);
	};
}

#endif

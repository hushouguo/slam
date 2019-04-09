/*
 * \file: Socket.h
 * \brief: Created by hushouguo at 16:29:33 Jun 21 2018
 */
 
#ifndef __SOCKET_H__
#define __SOCKET_H__

namespace net {
	enum {
		SOCKET_SERVER, SOCKET_CLIENT, SOCKET_CONNECTION
	};

	class EasynetInternal;
	class Socket {
		public:
			Socket(SOCKET s, EasynetInternal* easynet);
			~Socket();
			
		public:
			SOCKET fd() { return this->_fd; }
			int socket_type() { return this->_socket_type; }
			void socket_type(int value) { this->_socket_type = value; }
						
		public:
			bool receive();
			bool send(const SocketMessage* msg);
			bool send();

		private:
			SOCKET _fd = -1;
			int _socket_type = -1;
			EasynetInternal* _easynet = nullptr;
			Spinlocker _locker;
			std::list<const SocketMessage*> _sendQueue;

		private:			
			ByteBuffer _rbuffer, _wbuffer;
			ssize_t readBytes(Byte*, size_t);
			ssize_t sendBytes(const Byte*, size_t);
	};
}

#endif

/*
 * \file: SocketClient.cpp
 * \brief: Created by hushouguo at 01:57:59 Aug 09 2018
 */

#include "tnode.h"

#define CONNECT_TIMEOUT		10
//#define CONNECT_INTERVAL	5
//#define WAKE_PROCESS_SIGNAL	SIGRTMIN

BEGIN_NAMESPACE_TNODE {
	class SocketClientInternal : public SocketClient {
		public:
			SocketClientInternal();
			~SocketClientInternal();

		public:
			SOCKET fd() override { return this->_socket->fd(); }
			int socket_type() override { return this->_socket->socket_type(); }
			void socket_type(int value) override { this->_socket->socket_type(value); }			
			
		public:
			bool receive() override { return this->_socket->receive(); }
			bool send(const Servicemessage* message) override { return this->_socket->send(message); }
			bool send() override { return this->_socket->send(); }
			const Servicemessage* getMessage() override { return this->_socket->getMessage(); }

		public:
			bool connect(const char* address, int port) override;

		private:
			Socket* _socket = nullptr;
	};

	SocketClientInternal::SocketClientInternal() {
		this->_socket = SocketCreator::create(::socket(AF_INET, SOCK_STREAM, 0));
		assert(this->_socket);
		this->_socket->socket_type(SOCKET_CLIENT);		
	}

	SocketClient::~SocketClient() {}
	SocketClientInternal::~SocketClientInternal() {
		SafeDelete(this->_socket);
	}

	bool SocketClientInternal::connect(const char* address, int port) {
		CHECK_RETURN(this->fd() >= 0, false, "create socket failure: %d, %s", errno, strerror(errno));
		bool rc = connectSignal(this->fd(), address, port, CONNECT_TIMEOUT);
		if (rc) {
			rc = nonblocking(this->fd());
			CHECK_RETURN(rc, false, "nonblocking failure: %d, %s", errno, strerror(errno));
		}
		return rc;
	}
	
	SocketClient* SocketClientCreator::create() {
		return new SocketClientInternal();
	}
}

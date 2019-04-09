/*
 * \file: EasynetInternal.cpp
 * \brief: Created by hushouguo at 17:02:54 Apr 09 2019
 */

#define MAX_SOCKET		65536

namespace net {
	SOCKET EasynetInternal::createServer(const char* address, int port, size_t maxconn) {
		SocketServer* socketServer = SocketServerCreator::create();
		bool retval = socketServer->listen(address, port);
		if (!retval) {
			SafeDelete(socketServer);
			return -1;
		}
		assert(socketServer->fd() < MAX_SOCKET);
		assert(this->_sockets[socketServer->fd()] == nullptr);
		this->_sockets[socketServer->fd()] = socketServer;
		this->_poll.addSocket(socketServer->fd());
		return socketServer->fd();	
	}
	
	SOCKET EasynetInternal::createClient(const char* address, int port, size_t seconds) {
	}
	
	bool EasynetInternal::sendMessage(const SocketMessage* msg) {
	}
	
	const SocketMessage* EasynetInternal::getMessage() {
	}
	
	void EasynetInternal::closeSocket(SOCKET s) {
	}
	
	bool EasynetInternal::isActive(SOCKET s) {
	}
	
	void EasynetInternal::stop() {
		if (!this->isstop()) {
			this->_isstop = true;
			if (this->_thread && this->_thread->joinable()) {
				this->_thread->join();
			}
		}
	}

	EasynetInternal::EasynetInternal(std::function<int(const void*, size_t)> spliter) {
		this->_spliter = spliter;
	}
	
	Easynet::~Easynet() {}
	EasynetInternal::~EasynetInternal() {
		this->stop();
	}
	
	Easynet* Easynet::createInstance(std::function<int(const void*, size_t)> spliter) {
		return new EasynetInternal(spliter);
	}
}

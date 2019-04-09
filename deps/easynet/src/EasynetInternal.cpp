/*
 * \file: EasynetInternal.cpp
 * \brief: Created by hushouguo at 17:02:54 Apr 09 2019
 */

#define MAX_SOCKET		65536

namespace net {
	SocketMessage* allocateSocketMessage(size_t payload_len) {
		return (SocketMessage*) malloc(sizeof(SocketMessage) + payload_len);
	}
	
	void releaseSocketMessage(const SocketMessage* msg) {
		free(msg);
	}

	SOCKET EasynetInternal::createServer(const char* address, int port, size_t maxconn, std::function<int(const void*, size_t)> spliter) {
	}
	
	SOCKET EasynetInternal::createClient(const char* address, int port, size_t seconds, std::function<int(const void*, size_t)> spliter) {
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
	}

	EasynetInternal::EasynetInternal(std::function<int(const void*, size_t)> spliter) {
		this->_spliter(spliter);
	}
	
	Easynet::~Easynet() {}
	EasynetInternal::~EasynetInternal() {
	}
	
	Easynet* Easynet::createInstance(std::function<int(const void*, size_t)> spliter) {
		return new EasynetInternal(spliter);
	}
}

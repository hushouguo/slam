/*
 * \file: EasynetInternal.cpp
 * \brief: Created by hushouguo at 17:02:54 Apr 09 2019
 */

#include "Network.h"

namespace net {
	SOCKET EasynetInternal::createServer(const char* address, int port) {
		SocketServer* socketServer = SocketServerCreator::create(this);
		bool rc = socketServer->listen(address, port);
		if (!rc) {
			SafeDelete(socketServer);
			return -1;
		}
		assert(socketServer->fd() < MAX_SOCKET);
		assert(this->_sockets[socketServer->fd()] == nullptr);
		this->_sockets[socketServer->fd()] = socketServer;
		this->_poll->addSocket(socketServer->fd());
		return socketServer->fd();
	}
	
	SOCKET EasynetInternal::createClient(const char* address, int port) {
		SocketClient* socketClient = SocketClientCreator::create(this);
		bool rc = socketClient->connect(address, port);
		if (!rc) {
			SafeDelete(socketClient);
			return -1;
		}
		assert(socketClient->fd() < MAX_SOCKET);
		assert(this->_sockets[socketClient->fd()] == nullptr);
		this->_sockets[socketClient->fd()] = socketClient;
		this->_poll->addSocket(socketClient->fd());
		return socketClient->fd();
	}
	
	bool EasynetInternal::sendMessage(SOCKET s, void* msg) {
		assert(s < MAX_SOCKET);
		assert(isValidNetMessage(msg));
		Socket* socket = this->_sockets[s];
		CHECK_RETURN(socket, false, "Not found socket: %d when send msg", s);
		NetMessage* netmsg = (NetMessage*) msg;
		netmsg->fd = s;
		if (!socket->sendMessage(netmsg)) {
			this->closeSocket(s, "sendMessage error");
			return false;
		}
		return true;
	}
	
	const void* EasynetInternal::getMessage(SOCKET* s) {
		const NetMessage* msg = nullptr;
		this->_locker.lock();
		if (!this->_msgQueue.empty()) {
			msg = this->_msgQueue.front();
			this->_msgQueue.pop_front();
		}
		this->_locker.unlock();
		if (msg && s) {
			*s = msg->fd;
		}
		return msg;
	}
	
	void EasynetInternal::closeSocket(SOCKET s) {
		this->closeSocket(s, "outside");
	}
	
	bool EasynetInternal::isActive(SOCKET s) {
		return s < MAX_SOCKET && this->_sockets[s] != nullptr;
	}

	void EasynetInternal::closeSocket(SOCKET s, const char* reason) {
		Debug("Easynet, closeSocket: %d, reason: %s", s, reason);
		this->_poll->removeSocket(s);
		assert(s < MAX_SOCKET);
		Socket* socket = this->_sockets[s];
		CHECK_ALARM(socket, "Not found socket: %d", s);
		SafeDelete(socket);
		this->_sockets[s] = nullptr;
	}

	void EasynetInternal::socketRead(SOCKET s) {
		Socket* socket = this->_sockets[s];
		CHECK_RETURN(socket, void(0), "Not found socket: %d when socketRead", s);
		if (socket->socket_type() == SOCKET_SERVER) {
			SocketServer* socketServer = dynamic_cast<SocketServer*>(socket);
			assert(socketServer);
			for (SOCKET newfd = socketServer->accept(); newfd != -1; newfd = socketServer->accept()) {
				Socket* newsocket = SocketCreator::create(newfd, this);
				assert(this->_sockets[newfd] == nullptr);
				this->_sockets[newfd] = newsocket;
				this->_poll->addSocket(newfd);
				Debug("accept connection: %d", newfd);
			}
		}
		else {
			if (!socket->receive()) {
				this->closeSocket(s, "receive error");
			}
		}
	}
	
	void EasynetInternal::socketWrite(SOCKET s) {
		Socket* socket = this->_sockets[s];
		CHECK_RETURN(socket, void(0), "Not found socket: %d when socketWrite", s);
		if (!socket->send()) {
			this->closeSocket(s, "send error");
		}
	}
	
	void EasynetInternal::socketError(SOCKET s) {
		this->closeSocket(s, "poll error"); 
	}
	
	void EasynetInternal::run() {
		while (!this->isstop()) {
			this->_poll->run(-1);
		}
		Debug("Easynet exit, msgQueue: %ld", this->_msgQueue.size());
	}
	
	void EasynetInternal::stop() {
		if (!this->isstop()) {
			this->_isstop = true;
			this->_poll->stop();
			if (this->_threadWorker && this->_threadWorker->joinable()) {
				this->_threadWorker->join();
			}

			SafeDelete(this->_poll);
			SafeDelete(this->_threadWorker);
			
			for (auto& msg : this->_msgQueue) {
				releaseNetMessage(msg);
			}
			this->_msgQueue.clear();
		}
	}

	void* EasynetInternal::allocateMessage(size_t payload_len) {
		NetMessage* msg = allocateNetMessage(payload_len);
		msg->payload_len = payload_len;
		return msg;
	}
	
	void EasynetInternal::releaseMessage(const void* msg) {
		assert(isValidNetMessage(msg));
		releaseNetMessage((const NetMessage*) msg);
	}
	
	void EasynetInternal::setMessageContent(void* msg, const void* data, size_t len) {
		assert(isValidNetMessage(msg));
		NetMessage* netmsg = (NetMessage*) msg;
		assert(netmsg->size >= len);
		memcpy(netmsg->payload, data, len);
		netmsg->payload_len = len;
	}
	
	const void* EasynetInternal::getMessageContent(const void* msg, size_t* len) {
		assert(isValidNetMessage(msg));
		const NetMessage* netmsg = (const NetMessage*) msg;
		if (len) {
			*len = netmsg->payload_len;
		}
		return netmsg->payload;
	}

	SOCKET EasynetInternal::getMessageSocket(const void* msg) {
		assert(isValidNetMessage(msg));
		const NetMessage* netmsg = (const NetMessage*) msg;
		return netmsg->fd;
	}

	EasynetInternal::EasynetInternal(std::function<int(const void*, size_t)> spliter) {
		this->_spliter = spliter;
		this->_poll = new Poll(this);
		this->_threadWorker = new std::thread([this]() {
				this->run();
			});
	}
	
	Easynet::~Easynet() {}
	EasynetInternal::~EasynetInternal() {
		this->stop();
	}
	
	Easynet* Easynet::createInstance(std::function<int(const void*, size_t)> spliter) {
		return new EasynetInternal(spliter);
	}
}

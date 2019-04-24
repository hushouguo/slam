/*
 * \file: EasynetInternal.cpp
 * \brief: Created by hushouguo at 17:02:54 Apr 09 2019
 */

#include "Network.h"

#define VALID_SOCKET(s)	((s) >= 0 && (s) < MAX_SOCKET)

namespace net {
	SOCKET EasynetInternal::createServer(const char* address, int port) {
		SocketServer* socketServer = SocketServerCreator::create(this);
		bool rc = socketServer->listen(address, port);
		if (!rc) {
			SafeDelete(socketServer);
			return -1;
		}
		assert(VALID_SOCKET(socketServer->fd()));
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
		assert(VALID_SOCKET(socketClient->fd()));
		assert(this->_sockets[socketClient->fd()] == nullptr);
		this->_sockets[socketClient->fd()] = socketClient;
		this->_poll->addSocket(socketClient->fd());
		return socketClient->fd();
	}
	
	bool EasynetInternal::sendMessage(SOCKET s, const void* msg) {
		CHECK_RETURN(VALID_SOCKET(s), false, "invalid socket: %d", s);
		CHECK_RETURN(isValidNetMessage(msg), false, "invalid msg");
		CHECK_RETURN(this->_sockets[s], false, "Not found socket: %d when send msg", s);
		NetMessage* netmsg = (NetMessage*) msg;
		netmsg->fd = s;
		this->_sendlocker.lock();
		auto& list = this->_sendQueue[s];
		list.push_back(netmsg);
		this->_sendlocker.unlock();
		return this->poll()->setSocketPollout(s, true);
	}
	
	const void* EasynetInternal::getMessage(SOCKET* s) {
		const NetMessage* msg = nullptr;
		while (true) {
			this->_recvlocker.lock();
			if (!this->_recvQueue.empty()) {
				msg = this->_recvQueue.front();
				this->_recvQueue.pop_front();
			}
			this->_recvlocker.unlock();
			//
			// no more message
			if (!msg) {
				return nullptr;
			}

			if (!VALID_SOCKET(msg->fd)) {
				Error("getMessage invalid socket: %d", msg->fd);
				this->releaseMessage(msg);
				continue;
			}

			if (!isValidNetMessage(msg)) {
				Error("getMessage invalid netmsg");
				this->releaseMessage(msg);
				continue;
			}

			//
			// lost connection
			if (this->_sockets[msg->fd] == nullptr) {
				Alarm("getMessage found lost connection");
				this->releaseMessage(msg);
				continue;
			}

			if (s) {
				*s = msg->fd;
			}

			return msg;
		}
		return nullptr;
	}
	
	void EasynetInternal::closeSocket(SOCKET s) {
		this->closeSocket(s, "outside");
	}
	
	bool EasynetInternal::isActive(SOCKET s) {
		return VALID_SOCKET(s) && this->_sockets[s] != nullptr;
	}

	void EasynetInternal::closeSocket(SOCKET s, const char* reason) {
		Debug("Easynet, closeSocket: %d, reason: %s", s, reason);
		CHECK_RETURN(VALID_SOCKET(s), void(0), "invalid socket: %d", s);
		this->_poll->removeSocket(s);
		Socket* socket = this->_sockets[s];
		this->_sockets[s] = nullptr;
		CHECK_ALARM(socket, "Not found socket: %d", s);
		SafeDelete(socket);
	}

	void EasynetInternal::socketRead(SOCKET s) {
		assert(VALID_SOCKET(s));
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
		assert(VALID_SOCKET(s));
		Socket* socket = this->_sockets[s];
		CHECK_RETURN(socket, void(0), "Not found socket: %d when socketWrite", s);
		if (!socket->send()) {
			this->closeSocket(s, "send error");
		}
	}
	
	void EasynetInternal::socketError(SOCKET s) {
		int error = 0;
		socklen_t errlen = sizeof(error);
		if (getsockopt(s, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen) == 0) {
			Error("socketError: %s", strerror(error));
		}
		this->closeSocket(s, "poll error");
	}
	
	void EasynetInternal::run() {
		while (!this->isstop()) {
			this->_poll->run(-1);
		}
		Trace("Easynet exit, recvQueue: %ld", this->_recvQueue.size());
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

			//
			// cleanup recvQueue
			for (auto& msg : this->_recvQueue) {
				releaseNetMessage(msg);
			}
			this->_recvQueue.clear();

			//
			// cleanup writeQueue
			for (auto& i : this->_sendQueue) {
				auto& list = i.second;
				for (auto& msg : list) {
					releaseNetMessage(msg);
				}
			}
			this->_sendQueue.clear();
		}
	}

	const void* EasynetInternal::allocateMessage(size_t payload_len) {
		NetMessage* msg = allocateNetMessage(payload_len);
		msg->payload_len = payload_len;
		return msg;
	}
	
	void EasynetInternal::releaseMessage(const void* msg) {
		CHECK_RETURN(isValidNetMessage(msg), void(0), "invalid msg");
		releaseNetMessage((const NetMessage*) msg);
	}
	
	void EasynetInternal::setMessageContent(const void* msg, const void* data, size_t len) {
		CHECK_RETURN(isValidNetMessage(msg), void(0), "invalid msg");
		NetMessage* netmsg = (NetMessage*) msg;
		assert(netmsg->size >= len);
		memcpy(netmsg->payload, data, len);
		netmsg->payload_len = len;
	}
	
	const void* EasynetInternal::getMessageContent(const void* msg, size_t* len) {
		CHECK_RETURN(isValidNetMessage(msg), nullptr, "invalid msg");
		const NetMessage* netmsg = (const NetMessage*) msg;
		if (len) {
			*len = netmsg->payload_len;
		}
		return netmsg->payload;
	}

	SOCKET EasynetInternal::getMessageSocket(const void* msg) {
		CHECK_RETURN(isValidNetMessage(msg), -1, "invalid msg");
		const NetMessage* netmsg = (const NetMessage*) msg;
		return netmsg->fd;
	}

	EasynetInternal::EasynetInternal(std::function<int(const void*, size_t)> spliter, std::function<void()> notifymsg) {
		this->_spliter = spliter;
		this->_notifymsg = notifymsg;
		this->_poll = new Poll(this);
		this->_threadWorker = new std::thread([this]() {
				this->run();
			});
	}
	
	Easynet::~Easynet() {}
	EasynetInternal::~EasynetInternal() {
		this->stop();
	}
	
	Easynet* Easynet::createInstance(std::function<int(const void*, size_t)> spliter, std::function<void()> notifymsg) {
		return new EasynetInternal(spliter, notifymsg);
	}
}

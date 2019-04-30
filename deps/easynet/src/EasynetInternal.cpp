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
			return EASYNET_ILLEGAL_SOCKET;
		}
		assert(VALID_SOCKET(socketServer->fd()));
		assert(this->_sockets[socketServer->fd()] == nullptr);
		this->_sockets[socketServer->fd()] = socketServer;
		this->_poll->addSocket(socketServer->fd());
		++this->_totalConnections;
		return socketServer->fd();
	}
	
	SOCKET EasynetInternal::createClient(const char* address, int port, int seconds) {
		SocketClient* socketClient = SocketClientCreator::create(this);
		bool rc = socketClient->connect(address, port, seconds);
		if (!rc) {
			SafeDelete(socketClient);
			return EASYNET_ILLEGAL_SOCKET;
		}
		assert(VALID_SOCKET(socketClient->fd()));
		assert(this->_sockets[socketClient->fd()] == nullptr);
		this->_sockets[socketClient->fd()] = socketClient;
		this->_poll->addSocket(socketClient->fd());
		++this->_totalConnections;
		return socketClient->fd();
	}
	
	bool EasynetInternal::sendMessage(SOCKET s, const void* msg) {
		CHECK_RETURN(VALID_SOCKET(s), false, "invalid socket: %d", s);
		CHECK_RETURN(isValidNetMessage(msg), false, "invalid msg");
		CHECK_RETURN(this->_sockets[s], false, "Not found socket: %d when send msg", s);
		NetMessage* netmsg = (NetMessage*) msg;
		netmsg->fd = s;
		if (true) {
			SpinlockerGuard guard(&this->_sendlocker);
			auto& list = this->_sendQueue[s];
			list.push_back(netmsg);
		}
		return this->poll()->setSocketPollout(s, true);
	}	
	
	const NetMessage* EasynetInternal::getSendMessage(SOCKET s) {
		SpinlockerGuard guard(&this->_sendlocker);
		auto& list = this->_sendQueue[s];
		if (!list.empty()) {
			const NetMessage* netmsg = list.front();
			list.pop_front();
			return netmsg;
		}
		return nullptr;
	}
	
	void EasynetInternal::addReceiveMessage(const NetMessage* msg) {
		if (true) {
			SpinlockerGuard guard(&this->_recvlocker);
			this->_recvQueue.push_back(msg);
		}
		this->_messenger();
	}
	
	const void* EasynetInternal::receiveMessage(SOCKET* s) {
		const NetMessage* msg = nullptr;
		while (true) {
			if (true) {
				SpinlockerGuard guard(&this->_recvlocker);
				if (!this->_recvQueue.empty()) {
					msg = this->_recvQueue.front();
					this->_recvQueue.pop_front();
				}
			}
			
			if (!msg) {
				return nullptr;
			}

			if (!VALID_SOCKET(msg->fd) || !isValidNetMessage(msg)) {
				this->releaseMessage(msg);
				continue;
			}

#if EASYNET_RETURN_ACTIVE_MESSAGE
			//
			// connection is lost
			if (this->_sockets[msg->fd] == nullptr) {
				this->releaseMessage(msg);
				continue;
			}
#endif

			if (s) {
				*s = msg->fd;
			}

			return msg;
		}
		return nullptr;
	}
	
	void EasynetInternal::closeSocket(SOCKET s) {
		//this->closeSocket(s, "outside");
		Debug("Easynet, closeSocket: %d, outside", s);
		SafeClose(s);
	}
	
	bool EasynetInternal::isActive(SOCKET s) {
		return VALID_SOCKET(s) && this->_sockets[s] != nullptr;
	}

	size_t EasynetInternal::getQueueSize() {
		return this->_recvQueue.size();
	}
	
	size_t EasynetInternal::totalConnections() {
		return this->_totalConnections;
	}

	void EasynetInternal::closeSocket(SOCKET s, const char* reason) {
		Debug("Easynet, closeSocket: %d, reason: %s", s, reason);
		CHECK_RETURN(VALID_SOCKET(s), void(0), "invalid socket: %d", s);
		this->_poll->removeSocket(s);
		Socket* socket = this->_sockets[s];
		this->_sockets[s] = nullptr;
		CHECK_ALARM(socket, "Not found socket: %d", s);
		SafeDelete(socket);
		this->lostConnection(s);
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
				this->establishConnection(newfd);
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

			//
			// cleanup socketStates
			this->_socketStates.clear();

			//
			// cleanup Sockets
			for (auto& socket : this->_sockets) {
				SafeDelete(socket);
			}
			memset(this->_sockets, 0, sizeof(this->_sockets));

			//
			// reset totalConnections
			this->_totalConnections = 0;
		}
	}

	void EasynetInternal::establishConnection(SOCKET socket) {
		++this->_totalConnections;
		if (true) {
			SpinlockerGuard guard(&this->_statelocker);
			this->_socketStates.push_back(socket_state(socket, true));
		}
	}
	
	void EasynetInternal::lostConnection(SOCKET socket) {
		--this->_totalConnections;
		if (true) {
			SpinlockerGuard guard(&this->_statelocker);
			this->_socketStates.push_back(socket_state(socket, false));
		}
	}

	SOCKET EasynetInternal::getSocketState(bool& state) {
		SpinlockerGuard guard(&this->_statelocker);
		if (!this->_socketStates.empty()) {
			socket_state& ss = this->_socketStates.front();
			this->_socketStates.pop_front();
			state = ss.state;
			return ss.socket;
		}
		return EASYNET_ILLEGAL_SOCKET;
	}

	//
	//------------------------------------------------------------------------
	
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

	EasynetInternal::EasynetInternal(std::function<int(const void*, size_t)> spliter, std::function<void()> messenger) {
		this->_spliter = spliter;
		this->_messenger = messenger;
		this->_poll = new Poll(this);
		memset(this->_sockets, 0, sizeof(this->_sockets));
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

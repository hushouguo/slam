/*
 * \file: Socket.cpp
 * \brief: Created by hushouguo at 16:29:36 Jun 21 2018
 */

#include "Network.h"

namespace net {
	class SocketInternal : public Socket {
		public:
			SocketInternal(SOCKET s, EasynetInternal* easynet);
			~SocketInternal();
			
		public:
			SOCKET fd() override { return this->_fd; }
			int socket_type() override { return this->_socket_type; }
			void socket_type(int value) override { this->_socket_type = value; }
						
		public:
			bool receive() override;
			bool sendMessage(const NetMessage* msg) override;
			bool send() override;

		private:
			SOCKET _fd = -1;
			int _socket_type = -1;
			EasynetInternal* _easynet = nullptr;
			Spinlocker _locker;
			std::list<const NetMessage*> _sendQueue;

		private:			
			ByteBuffer _rbuffer, _wbuffer;
			ssize_t readBytes(Byte*, size_t);
			ssize_t sendBytes(const Byte*, size_t);
	};

	bool SocketInternal::receive() {
		while (true) {
			size_t readlen = 1024;
			ssize_t bytes = this->readBytes(this->_rbuffer.wbuffer(readlen), readlen);
			if (bytes < 0) {
				return false;	// error occupy when socketRead
			}
			
			if (bytes > 0) {
				this->_rbuffer.wlength(size_t(bytes));
			}
			
			if (size_t(bytes) < readlen) {
				break;	// read would block
			}
		}
		
		while (true) {
			// spliter: -1: error occupy, 0: incomplete package, > 0: len of package
			int rc = this->_easynet->spliter()(this->_rbuffer.rbuffer(), this->_rbuffer.size());
			if (rc == 0) {
				//return true;	// incomplete message
				break;
			}
			else if (rc < 0) {
				return false;	// illegal message
			}

			size_t msglen = size_t(rc);
			//assert(size_t(rc) <= this->_rbuffer.size());
			CHECK_RETURN(msglen <= this->_rbuffer.size(), false, "rc: %d overflow rbuffer size: %ld", rc, this->_rbuffer.size());
			NetMessage* msg = allocateNetMessage(msglen);
			msg->fd = this->fd();
			msg->payload_len = msglen;
			memcpy(msg->payload, this->_rbuffer.rbuffer(), msg->payload_len);
			this->_rbuffer.rlength(msglen);
			this->_easynet->pushMessage(msg);

			Debug("Socket: %d receive message: %ld", this->fd(), msglen);
		}
		
		return true;	
	}
		
	bool SocketInternal::sendMessage(const NetMessage* msg) {
		assert(msg->fd == this->fd());
		this->_locker.lock();
		this->_sendQueue.push_back(msg);
		this->_locker.unlock();
		return this->_easynet->poll()->setSocketPollout(this->fd(), true);	// set EPOLL_OUT
	}	

	bool SocketInternal::send() {
		if (this->_wbuffer.size() > 0) {
			ssize_t bytes = this->sendBytes(this->_wbuffer.rbuffer(), this->_wbuffer.size());
			if (bytes < 0) {
				return false;	// error occupy when socketSend
			}
			
			if (bytes > 0) {
				this->_wbuffer.rlength(size_t(bytes));
			}
		}

		if (this->_wbuffer.size() > 0) {
			return true;	// wbuffer did not send all, wait for next poll
		}

		const NetMessage* msg = nullptr;
		while (true) {
			this->_locker.lock();
			if (!this->_sendQueue.empty()) {
				msg = this->_sendQueue.front();
				this->_sendQueue.pop_front();
			}
			else {
				msg = nullptr;
			}
			this->_locker.unlock();

			if (!msg) {
				// remove EPOLL_OUT when send all messages over
				this->_easynet->poll()->setSocketPollout(this->fd(), false);
				break;	// no more message to send
			}

			assert(msg->fd == this->fd());

			ssize_t bytes = this->sendBytes((const Byte*) msg->payload, msg->payload_len);
			if (bytes < 0) {
				releaseSocketMessage(msg);
				return false;	// error occupy when socketSend
			}

			if (size_t(bytes) < msg->payload_len) {
				this->_wbuffer.append((const Byte*) msg->payload + bytes, msg->payload_len - bytes);
				releaseSocketMessage(msg);
				return true;	// send wouldblock, wait for next poll
			}

			releaseSocketMessage(msg);
		}
		
		return true;
	}

	//
	// < 0: error
	ssize_t SocketInternal::readBytes(Byte* buffer, size_t len) {
		ssize_t bytes = 0;
		while (true) {
			ssize_t rc = TEMP_FAILURE_RETRY(::recv(this->_fd, buffer + bytes, len - bytes, MSG_DONTWAIT | MSG_NOSIGNAL));			
			if (rc == 0) {
				Debug("lost Connection: %d", this->_fd);
				return -1; // lost connection
			}
			else if (rc < 0) {
				if (interrupted()) {
					continue;
				}				
				if (wouldblock()) {
					break; // no more data to read
				}
				CHECK_RETURN(false, rc, "socket receive error: %d, %s", errno, strerror(errno));
			}
			else {
				bytes += rc;
			}			
		}
		Debug("Socket: %d, readBytes: %ld, expect: %ld", this->fd(), bytes, len);
		return bytes;
	}

	//
	// < 0: error
	ssize_t SocketInternal::sendBytes(const Byte* buffer, size_t len) {
		ssize_t bytes = 0;
		while (len > size_t(bytes)) {
			ssize_t rc = TEMP_FAILURE_RETRY(::send(this->_fd, buffer + bytes, len - bytes, MSG_DONTWAIT | MSG_NOSIGNAL));
			if (rc == 0) {
				Debug("lost Connection:%d", this->_fd);
				return -1; // lost connection
			}
			else if (rc < 0) {
				if (interrupted()) { 
					continue; 
				}		
				if (wouldblock()) {
					break; // socket buffer is full
				}
				CHECK_RETURN(false, rc, "socket send error: %d, %s", errno, strerror(errno));
			}
			else {
				bytes += rc;
			}			
		}
		Debug("Socket: %d, sendBytes: %ld, expect: %ld", this->fd(), bytes, len);
		return bytes;
	}

	
	SocketInternal::SocketInternal(SOCKET s, EasynetInternal* easynet) {
		this->_fd = s;
		this->_socket_type = SOCKET_CONNECTION;
		this->_easynet = easynet;
	}

	Socket::~Socket() {}
	SocketInternal::~SocketInternal() {
		SafeClose(this->_fd);
		for (auto& msg : this->_sendQueue) {
			releaseSocketMessage(msg);
		}
		this->_sendQueue.clear();
	}

	Socket* SocketCreator::create(SOCKET s, EasynetInternal* easynet) {
		return new SocketInternal(s, easynet);
	}	
}


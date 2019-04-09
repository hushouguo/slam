/*
 * \file: Socket.cpp
 * \brief: Created by hushouguo at 16:29:36 Jun 21 2018
 */

#include "Network.h"

namespace net {

	bool Socket::receive() {
		size_t readlen = 960;
		ssize_t bytes = this->readBytes(this->_rbuffer.wbuffer(readlen), readlen);
		//CHECK_RETURN(bytes >= 0, false, "readBytes error");
		if (bytes < 0) {
			return false;
		}
		
		if (bytes > 0) {
			this->_rbuffer.wlength(size_t(bytes));
		}
		
		ByteBuffer& buffer = this->_rbuffer;
		while (true) {
			Socketmessage* rawmsg = (Socketmessage*) (buffer.rbuffer());
			if (buffer.size() < sizeof(rawmsg->len) || buffer.size() < rawmsg->len) {
				return true;	// not enough data
			}

			if (rawmsg->len < sizeof(Socketmessage)) {
				Error << "illegal package, len: " << rawmsg->len << ", at least: " << sizeof(Socketmessage);
				return false;	// illegal package
			}

			// got a new message
			Servicemessage* newmsg = allocate_message(rawmsg->len);	// Note: rawmsg->len - sizeof(Socketmessage)
			newmsg->fd = this->fd();
			memcpy(&newmsg->rawmsg, rawmsg, rawmsg->len);
			this->_rlist.push_back(newmsg);
			
			buffer.rlength(rawmsg->len);
		}
		
		return true;	
	}
		
	bool Socket::send(const SocketMessage* msg) {
		this->_slist.push_back(message);
		return this->send();
	}	

	bool Socket::send() {
		SpinlockerGuard guard(&this->_slocker);
					
		if (this->_wbuffer.size() > 0) {
			ssize_t bytes = this->sendBytes(this->_wbuffer.rbuffer(), this->_wbuffer.size());
			CHECK_RETURN(bytes >= 0, false, "sendBytes error");
			if (size_t(bytes) > 0) {
				this->_wbuffer.rlength(size_t(bytes));
			}
		}

		if (this->_wbuffer.size() > 0) {
			return true;	// wbuffer did not send all
		}

		while (!this->_slist.empty()) {
			const Servicemessage* message = this->_slist.pop_front();
			ssize_t bytes = this->sendBytes((const Byte*) &message->rawmsg, message->rawmsg.len);
			if (bytes < 0) {
				release_message(message);
				CHECK_RETURN(bytes >= 0, false, "sendBytes error");
			}

			if (size_t(bytes) < message->rawmsg.len) {
				this->_wbuffer.append((const Byte*) (&message->rawmsg) + bytes, message->rawmsg.len - bytes);
			}

			release_message(message);

			if (this->_wbuffer.size() > 0) {
				return true;	// send wouldblock
			}
		}			

		return true;
	}

	//
	// < 0: error
	ssize_t Socket::readBytes(Byte* buffer, size_t len) {
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
	ssize_t Socket::sendBytes(const Byte* buffer, size_t len) {
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
	
	Socket::Socket(SOCKET s, EasynetInternal* easynet) {
		this->_fd = s;
		this->_socket_type = SOCKET_CONNECTION;
		this->_easynet = easynet;
	}

	Socket::~Socket() {
		SafeClose(this->_fd);
		while (!this->_rlist.empty()) {
			const Servicemessage* message = this->_rlist.pop_front();
			release_message(message);
		}		
		while (!this->_slist.empty()) {
			const Servicemessage* message = this->_slist.pop_front();
			release_message(message);
		}
	}
}


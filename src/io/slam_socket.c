/*
 * \file: slam_socket.c
 * \brief: Created by hushouguo at 15:47:22 Jul 23 2019
 */

#include "slam.h"

#define SLAM_SOCKET_REUSE_ADDRESS		0
#define SLAM_SOCKET_REUSE_PORT			1

bool slam_interrupted() {
	return errno == EINTR;
}

bool slam_wouldblock() {
	return errno == EAGAIN || errno == EWOULDBLOCK;
}

bool slam_connectionlost() {
	return errno == ECONNRESET || errno == ENOTCONN || errno == ESHUTDOWN || errno == ECONNABORTED || errno == EPIPE;
}

bool slam_socket_reuseable_address(SOCKET s) {
	int one = 1;
	return setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void*)&one, sizeof(one)) == 0;
}

bool slam_socket_reuseable_port(SOCKET s) {
	int val = 1;
	return setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) == 0;
}

bool slam_socket_blocking(SOCKET s) {
	return fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK) != -1;
}

bool slam_socket_nonblocking(SOCKET s) {
	return fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) != -1;
}

bool slam_socket_nodelay(SOCKET s, bool enable_nodelay) {
	int on = enable_nodelay;
	return setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on)) == 0;
}


slam_socket_t* slam_socket_newfd(SOCKET connfd, int socket_type) {
	slam_socket_t * socket = (slam_socket_t *) slam_malloc(sizeof(slam_socket_t));
	socket->fd = connfd;
	socket->type = socket_type;
	socket->readmessage = nullptr;
	socket->sendmq = slam_message_queue_new(__slam_main->socket_send_queue_size);	
	return socket;
}

slam_socket_t* slam_socket_new() {
	SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET) {
		return NULL;
	}
	return slam_socket_newfd(fd, SOCKET_NONE);
}

void slam_socket_delete(slam_socket_t* socket) {
	slam_close(socket->fd);
	if (socket->readmessage) {
	    slam_message_delete(socket->readmessage);
	}
	slam_message_queue_delete(socket->sendmq);
	slam_free(socket);
}

bool slam_socket_bind(slam_socket_t* socket, const char* address, int port) {
	bool rc;
	int retval;
	struct sockaddr_in sockaddr;
	
	assert(socket);
	assert(socket->fd != INVALID_SOCKET);
	
#if SLAM_SOCKET_REUSE_ADDRESS		
	rc = slam_socket_reuseable_address(socket->fd);
	CHECK_RETURN(rc, false, "reuseable address error: %d, %s", errno, errstring(errno));
#endif

#if SLAM_SOCKET_REUSE_PORT
	rc = slam_socket_reuseable_port(socket->fd);
	CHECK_RETURN(rc, false, "reuseable port error: %d, %s", errno, errstring(errno));
#endif

	rc = slam_socket_nonblocking(socket->fd);
	CHECK_RETURN(rc, false, "nonblocking error: %d, %s", errno, errstring(errno));
		
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	inet_aton(address, &(sockaddr.sin_addr));
	sockaddr.sin_port = htons(port);
	
	retval = bind(socket->fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	CHECK_RETURN(retval == 0, false, "bind error: %d, %s", errno, errstring(errno));
	
	retval = listen(socket->fd, SOMAXCONN);
	CHECK_RETURN(retval == 0, false, "listen error: %d, %s", errno, errstring(errno));

	assert(socket->type == SOCKET_NONE);
	socket->type = SOCKET_SERVER;
	
	return true;
}

bool slam_socket_connect(slam_socket_t* socket, const char* address, int port, int timeout) {
	struct sigaction act, oldact;
	struct sockaddr_in sockaddr;
	bool result = true;

	assert(socket);
	assert(socket->fd != INVALID_SOCKET);
	
	sigemptyset(&act.sa_mask);	
	sigaddset(&act.sa_mask, SIGALRM);  
	act.sa_flags = SA_INTERRUPT; /* The system call that is interrupted by this signal will not be restarted automatically */
	act.sa_handler = nullptr;
	sigaction(SIGALRM, &act, &oldact);
	
	alarm(timeout); /* timeout is N seconds */
	
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	inet_aton(address, &(sockaddr.sin_addr));
	sockaddr.sin_port = htons(port);
	
	slam_socket_blocking(socket->fd);
	if (connect(socket->fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in)) < 0) {
		if (errno == EINTR) {
			log_error("connectSignal timeout");
		}
		else {
			log_error("connectSignal error: %d, %s", errno, errstring(errno));
		}
		result = false;
	}
	slam_socket_nonblocking(socket->fd);
	
	alarm(0);
	sigaction(SIGALRM, &oldact, nullptr);

	if (result) {
		assert(socket->type == SOCKET_NONE);
		socket->type = SOCKET_CLIENT;
	}
	
	return result;
}

slam_socket_t* slam_socket_accept(slam_socket_t* socket) {
	assert(socket);
	assert(socket->fd != INVALID_SOCKET);
	
	while (true) {
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		SOCKET connfd = accept(socket->fd, (struct sockaddr*)&addr, &len);
		if (connfd < 0) {
			if (slam_interrupted()) {
				continue;
			}
			
			if (slam_wouldblock()) {
				return nullptr; /* no more connection */
			}
			
			CHECK_RETURN(false, nullptr, "accept error: %d,%s", errno, errstring(errno));
		}
		if (!slam_socket_nonblocking(connfd)) {
		    log_error("set fd: %d nonblocking error: %d, %s", connfd, errno, errstring(errno));
		    slam_close(connfd);
		}
		else {		
    		return slam_socket_newfd(connfd, SOCKET_CONNECTION);
		}
	}
	
	return nullptr;
}

ssize_t slam_socket_read_bytes(slam_socket_t* socket, void* buf, size_t bufsize) {
    size_t readbytes = 0;
	while (readbytes < bufsize) {
		ssize_t len = TEMP_FAILURE_RETRY(
			recv(socket->fd,
    			    (byte_t*)buf + readbytes,
    			    bufsize - readbytes,
    				MSG_DONTWAIT | MSG_NOSIGNAL)
				);
				
		if (len == 0) {
			return SLAM_ERROR; /* lost connection */
		}
		else if (len < 0) {
			if (slam_interrupted()) {
				continue;
			}
			
			if (slam_wouldblock()) {
				break; /* no more data to read */
			}
			
			CHECK_RETURN(false, SLAM_ERROR, "socket recv error: %d, %s", errno, errstring(errno));
		}
		else {
			readbytes += len;
		}
	}	
	//log_debug("socket: %d read bytes: %ld", socket->fd, readbytes);
	return readbytes;
}

//
// twice read operation
bool slam_socket_poll_read(slam_socket_t* socket, slam_message_queue_t* readmq) {
    while (true) {
        if (socket->readmessage == nullptr) {
            msglen_t len = 0;
            ssize_t readbytes = slam_socket_read_bytes(socket, &len, sizeof(len));
            if (readbytes < 0) {  // read error
                return false;
            }
            if (readbytes == 0) { // EAGAIN
                return true;
            }
            // NOTE: read len of message not enough, as a reading error
            CHECK_RETURN(readbytes == sizeof(len), false, "try to read len error: %ld", readbytes);
            // check len is valid
            CHECK_RETURN(len >= PACKAGE_MIN_SIZE, false, "len: %d, package.min.size: %ld", len, PACKAGE_MIN_SIZE);
            CHECK_RETURN(len <  PACKAGE_MAX_SIZE, false, "len: %d, package.max.size: %ld", len, PACKAGE_MAX_SIZE);
            // allocate new message
            socket->readmessage = slam_message_new(socket->fd, MESSAGE_TYPE_PROTOBUF_PACKAGE, len);
            // store len to readmessage
            memcpy(&socket->readmessage->buf[socket->readmessage->bytesize], &len, sizeof(len));
            socket->readmessage->bytesize += sizeof(len);
        }
        assert(socket->readmessage->bytesize <= socket->readmessage->bufsize);
        // bufsize is msglen
        if (socket->readmessage->bytesize == socket->readmessage->bufsize) { 
            if (!slam_message_queue_push_back(readmq, socket->readmessage)) {
                return false; // readmq is full
            }
            socket->readmessage = nullptr;
        }
        else {
            ssize_t readbytes = slam_socket_read_bytes(
                                    socket, 
                                    &socket->readmessage->buf[socket->readmessage->bytesize], 
                                    socket->readmessage->bufsize - socket->readmessage->bytesize
                                );
            if (readbytes < 0) { // read error
                return false;
            }
            if (readbytes == 0) { // EAGAIN
                return true;
            }
            socket->readmessage->bytesize += readbytes;
            assert(socket->readmessage->bytesize <= socket->readmessage->bufsize);
        }
    }
    return false;
}

bool slam_socket_poll_write(slam_socket_t* socket) {
	while (!slam_message_queue_empty(socket->sendmq)) {
	    slam_message_t* message = slam_message_queue_front(socket->sendmq);
	    assert(message->bytesize < message->bufsize);
	    
		ssize_t len = TEMP_FAILURE_RETRY(
			send(socket->fd, 
			        &message->buf[message->bytesize],
			        message->bufsize - message->bytesize,
    				MSG_DONTWAIT | MSG_NOSIGNAL)
				);
				
		if (len == 0) {
			return false; /* lost connection */
		}
		else if (len < 0) {
			if (slam_interrupted()) { 
				continue; 
			}
			
			if (slam_wouldblock()) {
				break; /* socket buffer is full */
			}
			
			CHECK_RETURN(false, false, "socket send error: %d, %s", errno, errstring(errno));
		}
		else {
			//log_debug("socket: %d send bytes: %ld", socket->fd, len);
            message->bytesize += len;
            assert(message->bytesize <= message->bufsize);
    	    if (message->bytesize == message->bufsize) {
    	        slam_message_queue_pop_front(socket->sendmq);
    	        slam_message_delete(message);
    	    }
		}
	}
	return true;
}

bool slam_socket_write_message(slam_socket_t* socket, const slam_message_t* message) {
    if (!slam_message_queue_push_back(socket->sendmq, message)) {
        return false; // sendmq is full
    }
    return true;
}


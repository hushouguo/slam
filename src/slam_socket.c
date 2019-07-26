/*
 * \file: slam_socket.c
 * \brief: Created by hushouguo at 15:47:22 Jul 23 2019
 */

#include "slam.h"

#define SLAM_SOCKET_REUSE_ADDRESS		1
#define SLAM_SOCKET_REUSE_PORT			0

#define SLAM_SOCKET_RECV_BUFFER_SIZE	64*KB
#define SLAM_SOCKET_SEND_BUFFER_SIZE	64*KB

#define SLAM_SOCKET_READ_SIZE			1*KB

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


slam_socket_t* slam_socket_newfd(SOCKET connfd) {
	slam_socket_t * socket = (slam_socket_t *) malloc(sizeof(slam_socket_t));
	socket->fd = connfd;
	socket->type = SOCKET_NONE;
	socket->rbuffer = slam_byte_buffer_new(SLAM_SOCKET_RECV_BUFFER_SIZE);
	socket->wbuffer = slam_byte_buffer_new(SLAM_SOCKET_SEND_BUFFER_SIZE);
	return socket;
}

slam_socket_t* slam_socket_new() {
	SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET) {
		return NULL;
	}
	return slam_socket_newfd(fd);
}

void slam_socket_delete(slam_socket_t* socket) {
	slam_close(socket->fd);
	slam_byte_buffer_delete(socket->rbuffer);
	slam_byte_buffer_delete(socket->wbuffer);
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
	CHECK_RETURN(rc, false, "reuseable address error: %d, %s", errno, strerror(errno));
#endif

#if SLAM_SOCKET_REUSE_PORT
	rc = slam_socket_reuseable_port(socket->fd);
	CHECK_RETURN(rc, false, "reuseable port error: %d, %s", errno, strerror(errno));
#endif

	rc = slam_socket_nonblocking(socket->fd);
	CHECK_RETURN(rc, false, "nonblocking error: %d, %s", errno, strerror(errno));
		
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	inet_aton(address, &(sockaddr.sin_addr));
	sockaddr.sin_port = htons(port);
	
	retval = bind(socket->fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	CHECK_RETURN(retval == 0, false, "bind error: %d, %s", errno, strerror(errno));
	
	retval = listen(socket->fd, SOMAXCONN);
	CHECK_RETURN(retval == 0, false, "listen error: %d, %s", errno, strerror(errno));

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
			Error("connectSignal timeout");
		}
		else {
			Error("connectSignal error: %d, %s", errno, strerror(errno));
		}
		result = false;
	}
	slam_socket_nonblocking(socket->fd);
	
	alarm(0);
	sigaction(SIGALRM, &oldact, nullptr);

	assert(socket->type == SOCKET_NONE);
	socket->type = SOCKET_CLIENT;
	
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
			
			CHECK_RETURN(false, nullptr, "accept error: %d,%s", errno, strerror(errno));
		}
		slam_socket_t* newsocket = slam_socket_newfd(connfd);
		assert(newsocket->type == SOCKET_NONE);
		newsocket->type = SOCKET_CONNECTION;
		return newsocket;
	}
	
	return nullptr;
}

ssize_t slam_socket_read(slam_socket_t* socket) {
	size_t readbytes = 0;
	while (true) {
		ssize_t len = TEMP_FAILURE_RETRY(
			recv(socket->fd, 
				slam_byte_buffer_writebuffer(socket->rbuffer, SLAM_SOCKET_READ_SIZE), 
				SLAM_SOCKET_READ_SIZE, 
				MSG_DONTWAIT | MSG_NOSIGNAL)
				);
				
		if (len == 0) {
			return -1; /* lost connection */
		}
		else if (len < 0) {
			if (slam_interrupted()) {
				continue;
			}
			
			if (slam_wouldblock()) {
				break; /* no more data to read */
			}
			
			CHECK_RETURN(false, -1, "socket recv error: %d, %s", errno, strerror(errno));
		}
		else {
			readbytes += len;
			slam_byte_buffer_writelength(socket->rbuffer, len);
		}
	}	
	return readbytes;
}

ssize_t slam_socket_write_buffer(slam_socket_t* socket) {
	size_t writebytes = 0;
	while (slam_byte_buffer_size(socket->wbuffer) > 0) {
		ssize_t len = TEMP_FAILURE_RETRY(
			send(socket->fd, 
				slam_byte_buffer_readbuffer(socket->wbuffer),
				slam_byte_buffer_size(socket->wbuffer),
				MSG_DONTWAIT | MSG_NOSIGNAL)
				);
				
		if (len == 0) {
			return -1; /* lost connection */
		}
		else if (len < 0) {
			if (slam_interrupted()) { 
				continue; 
			}
			
			if (slam_wouldblock()) {
				break; /* socket buffer is full */
			}
			
			CHECK_RETURN(false, -1, "socket send error: %d, %s", errno, strerror(errno));
		}
		else {
			writebytes += len;
			slam_byte_buffer_readlength(socket->wbuffer, len);
		}
	}	
	return writebytes;
}

ssize_t slam_socket_write(slam_socket_t* socket, const void* buf, size_t bufsize) {
	slam_byte_buffer_write(socket->wbuffer, buf, bufsize);
	return slam_socket_write_buffer(socket);
}


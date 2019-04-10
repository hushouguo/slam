/*
 * \file: SocketMessage.cpp
 * \brief: Created by hushouguo at 17:28:58 Apr 09 2019
 */

#include "Network.h"

namespace net {
	SocketMessage* allocateSocketMessage(size_t payload_len) {
		return (SocketMessage*) malloc(sizeof(SocketMessage) + payload_len);
	}
	
	void releaseSocketMessage(const SocketMessage* msg) {
		free((void*) msg);
	}
}

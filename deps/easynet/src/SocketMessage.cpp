/*
 * \file: NetMessage.cpp
 * \brief: Created by hushouguo at 17:28:58 Apr 09 2019
 */

#include "Network.h"

namespace net {
	NetMessage* allocateSocketMessage(size_t payload_len) {
		return (NetMessage*) malloc(sizeof(NetMessage) + payload_len);
	}
	
	void releaseSocketMessage(const NetMessage* msg) {
		free((void*) msg);
	}
}

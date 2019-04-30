/*
 * \file: NetMessage.cpp
 * \brief: Created by hushouguo at 17:28:58 Apr 09 2019
 */

#include "Network.h"

#define MAGIC_MESSAGE	0x0a0a0a0a	

namespace net {
	NetMessage* allocateNetMessage(size_t payload_len) {
		NetMessage* msg = (NetMessage*) malloc(sizeof(NetMessage) + payload_len);
		msg->magic = MAGIC_MESSAGE;
		msg->size = payload_len;
		return msg;
	}
	
	void releaseNetMessage(const NetMessage* msg) {
		assert(msg->magic == MAGIC_MESSAGE);
		free((void*) msg);
	}

	bool isValidNetMessage(const void* msg) {
		const NetMessage* netmsg = (const NetMessage*) msg;
		return netmsg->magic == MAGIC_MESSAGE;
	}
}

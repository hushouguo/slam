/*
 * \file: NetMessage.h
 * \brief: Created by hushouguo at 13:35:35 Apr 10 2019
 */
 
#ifndef __NETMESSAGE_H__
#define __NETMESSAGE_H__

namespace net {
	struct NetMessage {
		uint32_t magic;
		SOCKET fd;
		size_t size;
		size_t payload_len;
		char payload[0];
	};

	NetMessage* allocateNetMessage(size_t payload_len);
	void releaseNetMessage(const NetMessage*);
	bool isValidNetMessage(void* msg);
}

#endif

/*
 * \file: NetMessage.h
 * \brief: Created by hushouguo at 13:35:35 Apr 10 2019
 */
 
#ifndef __NETMESSAGE_H__
#define __NETMESSAGE_H__

namespace net {
	struct NetMessage {
		SOCKET fd;
		size_t payload_len;
		char payload[0];
	};

	NetMessage* allocateNetMessage(size_t payload_len);
	void releaseNetMessage(const NetMessage*);
}

#endif
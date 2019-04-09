/*
 * \file: Helper.h
 * \brief: Created by hushouguo at 20:06:34 Aug 09 2018
 */
 
#ifndef __HELPER_H__
#define __HELPER_H__

#define MAGIC	0x12345678	
#define INTERNAL_SIGN	0x77abc3694dfb225e

namespace net {
	bool interrupted();
	bool wouldblock();
	bool connectionlost();
	bool reuseableAddress(SOCKET s);
	bool reuseablePort(SOCKET s);
	bool blocking(SOCKET s);
	bool nonblocking(SOCKET s);	
	bool nodelay(SOCKET s, bool enable);	
	bool connectSignal(SOCKET s, const char* address, int port, int seconds);
	bool connectSelect(SOCKET s, const char* address, int port, int milliseconds);
}

#endif

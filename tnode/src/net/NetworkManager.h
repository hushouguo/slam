/*
 * \file: NetworkManager.h
 * \brief: Created by hushouguo at 16:54:12 Apr 02 2019
 */
 
#ifndef __NETWORKMANAGER_H__
#define __NETWORKMANAGER_H__

BEGIN_NAMESPACE_TNODE {
	class NetworkManager {
		public:
			void init();
			void stop();
			void run();
			inline Easynet* easynet() { return this->_easynet; }

		public:
			SOCKET createServer(const char* name, const char*, int);
			SOCKET createClient(const char* name, const char*, int);
			SOCKET findSocket(const char* name);
			
		private:
			Easynet* _easynet = nullptr;
			LockfreeMap<std::string, SOCKET> _registeSockets;
	};
}

#define sNetworkManager tnode::Singleton<tnode::NetworkManager>::getInstance()

#endif

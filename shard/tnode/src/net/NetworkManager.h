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
			void wakeup();
			
		private:
			Easynet* _easynet = nullptr;
			void dispatchSocketState();
			void dispatchMessage();

		private:
			std::mutex _mtx;
			std::condition_variable _cond;
	};
}

#define sNetworkManager tnode::Singleton<tnode::NetworkManager>::getInstance()

#endif

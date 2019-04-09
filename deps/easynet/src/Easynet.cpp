/*
 * \file: Easynet.cpp
 * \brief: Created by hushouguo at 10:29:59 Jul 06 2018
 */

#include "Easynet.h"

#define MAX_SOCKET		65536

namespace net {
	SocketMessage* allocateSocketMessage(size_t payload_len) {
		return (SocketMessage*) malloc(sizeof(SocketMessage) + payload_len);
	}
	
	void releaseSocketMessage(const SocketMessage* msg) {
		free(msg);
	}

	class Spinlocker {
		public:
			void lock() {
				while (this->_locker.test_and_set(std::memory_order_acquire));
			}

			bool trylock() {
				return !this->_locker.test_and_set(std::memory_order_acquire);// set OK, return false
			}

			void unlock() {
				this->_locker.clear(std::memory_order_release);
			}
		private:
			std::atomic_flag _locker = ATOMIC_FLAG_INIT;
	};

	class EasynetInternal : public Easynet {
		public:
			EasynetInternal(std::function<int(const void*, size_t)> spliter);
			~EasynetInternal();

		public:
			SOCKET createServer(const char*, int, size_t, std::function<int(const void*, size_t)>) override;
			SOCKET createClient(const char*, int, size_t, std::function<int(const void*, size_t)>) override;
			bool sendMessage(const SocketMessage* msg) override;
			const SocketMessage* getMessage() override;
			void closeSocket(SOCKET) override;
			bool isActive(SOCKET) override;
			void stop() override;

		public:
		
		private:
			Poll _poll;
			Socket* _sockets[MAX_SOCKET];
			Spinlocker _locker;
			std::list<const SocketMessage*> _msgQueue;
			inline void pushMessage(const SocketMessage* msg) {
				this->_locker.lock();
				this->_msgQueue.push_back(msg);
				this->_locker.unlock();
			}
			inline const SocketMessage* popMessage() {
				const SocketMessage* msg = nullptr;
				this->_locker.lock();
				if (!this->_msgQueue.empty()) {
					msg = this->_msgQueue.front();
					this->_msgQueue.pop_front();
				}
				this->_locker.unlock();
				return msg;
			}
			std::function<int(const void*, size_t)> _spliter;
	};

	SOCKET EasynetInternal::createServer(const char* address, int port, size_t maxconn, std::function<int(const void*, size_t)> spliter) {
	}
	
	SOCKET EasynetInternal::createClient(const char* address, int port, size_t seconds, std::function<int(const void*, size_t)> spliter) {
	}
	
	bool EasynetInternal::sendMessage(const SocketMessage* msg) {
	}
	
	const SocketMessage* EasynetInternal::getMessage() {
	}
	
	void EasynetInternal::closeSocket(SOCKET s) {
	}
	
	bool EasynetInternal::isActive(SOCKET s) {
	}
	
	void EasynetInternal::stop() {
	}

	EasynetInternal::EasynetInternal(std::function<int(const void*, size_t)> spliter) {
		this->_spliter(spliter);
	}
	
	Easynet::~Easynet() {}
	EasynetInternal::~EasynetInternal() {
	}
	
	Easynet* Easynet::createInstance(std::function<int(const void*, size_t)> spliter) {
		return new EasynetInternal(spliter);
	}
}

#endif

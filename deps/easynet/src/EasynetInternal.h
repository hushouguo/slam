/*
 * \file: EasynetInternal.h
 * \brief: Created by hushouguo at 17:02:46 Apr 09 2019
 */
 
#ifndef __EASYNETINTERNAL_H__
#define __EASYNETINTERNAL_H__

#define MAX_SOCKET		65536

namespace net {
	class EasynetInternal : public Easynet {
		public:
			EasynetInternal(std::function<int(const void*, size_t)> spliter);
			~EasynetInternal();

		public:
			SOCKET createServer(const char*, int) override;
			SOCKET createClient(const char*, int) override;
			bool sendMessage(SOCKET s, void* msg) override;
			const void* getMessage(SOCKET*) override;
			void closeSocket(SOCKET) override;
			bool isActive(SOCKET) override;
			void stop() override;

		public:
			void* allocateMessage(size_t payload_len) override;
			void releaseMessage(const void* msg) override;
			void setMessageContent(void* msg, const void* data, size_t len) override;
			const void* getMessageContent(const void* msg, size_t* len) override;
			SOCKET getMessageSocket(const void* msg) override;
		
		public:
			void run();
			void socketRead(SOCKET);
			void socketWrite(SOCKET);
			void socketError(SOCKET);

		public:
			inline bool isstop() { return this->_isstop; }
			inline const std::function<int(const void*, size_t)>& spliter() {
				return this->_spliter;
			}
			inline Poll* poll() { return this->_poll; }
			inline void pushMessage(const NetMessage* msg) {
				this->_locker.lock();
				this->_msgQueue.push_back(msg);
				this->_locker.unlock();
			}

		private:
			bool _isstop = false;
			std::thread* _threadWorker = nullptr;
			std::function<int(const void*, size_t)> _spliter;
			
		private:
			Poll* _poll = nullptr;
			Socket* _sockets[MAX_SOCKET];
			Spinlocker _locker;
			std::list<const NetMessage*> _msgQueue;
			void closeSocket(SOCKET, const char* reason);
	};
}

#endif

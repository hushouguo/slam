/*
 * \file: EasynetInternal.h
 * \brief: Created by hushouguo at 17:02:46 Apr 09 2019
 */
 
#ifndef __EASYNETINTERNAL_H__
#define __EASYNETINTERNAL_H__

namespace net {
	class EasynetInternal : public Easynet {
		public:
			EasynetInternal(std::function<int(const void*, size_t)> spliter);
			~EasynetInternal();

		public:
			SOCKET createServer(const char*, int) override;
			SOCKET createClient(const char*, int) override;
			bool sendMessage(const SocketMessage* msg) override;
			const SocketMessage* getMessage() override;
			void closeSocket(SOCKET) override;
			bool isActive(SOCKET) override;
			void stop() override;

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
			inline void pushMessage(const SocketMessage* msg) {
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
			std::list<const SocketMessage*> _msgQueue;
			void closeSocket(SOCKET, const char* reason);
	};
}

#endif

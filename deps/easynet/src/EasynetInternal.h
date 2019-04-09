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
			SOCKET createServer(const char*, int, size_t, std::function<int(const void*, size_t)>) override;
			SOCKET createClient(const char*, int, size_t, std::function<int(const void*, size_t)>) override;
			bool sendMessage(const SocketMessage* msg) override;
			const SocketMessage* getMessage() override;
			void closeSocket(SOCKET) override;
			bool isActive(SOCKET) override;
			void stop() override;

		public:
			void run();

		private:
			std::mutex _mtx;
			std::condition_variable _cond;
			std::thread* _thread = nullptr;

		public:
			inline bool isstop() { return this->_isstop; }
			inline std::function<int(const void*, size_t)>& spliter() {
				return this->_spliter;
			}
			inline Poll& poll() { return this->_poll; }

		private:
			bool _isstop = false;
			std::function<int(const void*, size_t)> _spliter;
			
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
	};
}

#endif

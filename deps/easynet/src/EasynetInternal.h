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
			EasynetInternal(std::function<int(const void*, size_t)> spliter, std::function<void()> messenger);
			~EasynetInternal();

		public:
			SOCKET createServer(const char*, int) override;
			SOCKET createClient(const char*, int, int seconds) override;
			bool sendMessage(SOCKET s, const void* msg) override;
			const void* receiveMessage(SOCKET*) override;
			void closeSocket(SOCKET) override;
			bool isActive(SOCKET) override;
			void stop() override;
			size_t getQueueSize() override;
			size_t totalConnections() override;
			SOCKET getSocketState(bool* state) override;

		public:
			const void* allocateMessage(size_t payload_len) override;
			void releaseMessage(const void* msg) override;
			void setMessageContent(const void* msg, const void* data, size_t len) override;
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
			void addReceiveMessage(const NetMessage* msg);
			const NetMessage* getSendMessage(SOCKET s);

		private:
			bool _isstop = false;
			std::thread* _threadWorker = nullptr;
			std::function<int(const void*, size_t)> _spliter;
			std::function<void()> _messenger;
			void closeSocket(SOCKET, const char* reason);
			
		private:
			Poll* _poll = nullptr;
			size_t _totalConnections = 0;
			Socket* _sockets[MAX_SOCKET];

		private:
			Spinlocker _recvlocker;
			std::list<const NetMessage*> _recvQueue;
			Spinlocker _sendlocker;
			std::unordered_map<SOCKET, std::list<const NetMessage*>> _sendQueue;

		private:
			struct socket_state {
				SOCKET socket;
				bool state;	// true: establish, false: lost
				socket_state(SOCKET s, bool value) : socket(s), state(value) {}
			};
			Spinlocker _statelocker;
			std::list<socket_state> _socketStates;
			void establishConnection(SOCKET socket);
			void lostConnection(SOCKET socket);
	};
}

#endif

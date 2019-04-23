/*
 * \file: Poll.h
 * \brief: Created by hushouguo at 14:39:25 Jun 28 2018
 */
 
#ifndef __POLL_H__
#define __POLL_H__

#define NM_POLL_EVENT		128

namespace net {
	class EasynetInternal;
	class Poll {
		public:
			Poll(EasynetInternal* easynet);
			~Poll();

		public:
			void wakeup();
			void run(int milliseconds);
			
		public:
			bool addSocket(SOCKET s);
			bool removeSocket(SOCKET s);
			bool setSocketPollin(SOCKET s, bool value);
			bool setSocketPollout(SOCKET s, bool value);

		private:
			int _epfd = -1, _wakefd = -1;
			struct epoll_event _events[NM_POLL_EVENT];
			EasynetInternal* _easynet = nullptr;
	};
}

#endif

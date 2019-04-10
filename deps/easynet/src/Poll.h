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
			void stop();
			void run(int milliseconds);
			
		public:
			bool addSocket(SOCKET s);
			bool removeSocket(SOCKET s);
			bool setSocketPollout(SOCKET s, bool value);

		private:
			bool _isstop = false;
			int _epfd = -1;
			struct epoll_event _events[NM_POLL_EVENT];
			EasynetInternal* _easynet = nullptr;
	};
}

#endif

/*
 * \file: Poll.cpp
 * \brief: Created by hushouguo at 14:39:56 Jun 28 2018
 */

#include "Network.h"

namespace net {
	Poll::Poll(EasynetInternal* easynet) {
		this->_easynet = easynet;
		this->_epfd = epoll_create(NM_POLL_EVENT); /* `NM_POLL_EVENT` is just a hint for the kernel */
		memset(this->_events, 0, sizeof(this->_events));
		this->_wakefd = open("/dev/null", O_RDWR);
	}
		
	Poll::~Poll() {
		SafeClose(this->_wakefd);
	}

	bool Poll::addSocket(SOCKET s) {
		//System << "Poll::addSocket: " << s;
		struct epoll_event ee;
		//ee.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR;
		ee.events = EPOLLET | EPOLLIN | EPOLLERR;
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		int rc = epoll_ctl(this->_epfd, EPOLL_CTL_ADD, s, &ee);		
		CHECK_RETURN(rc == 0, false, "epoll_add error: %d, %s", errno, strerror(errno));
		return true;
	}

	bool Poll::removeSocket(SOCKET s) {
		//System << "Poll::removeSocket: " << s;
		struct epoll_event ee;
		ee.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR;
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		/* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
		int rc = epoll_ctl(this->_epfd, EPOLL_CTL_DEL, s, &ee);
		CHECK_RETURN(rc == 0, false, "epoll_del error: %d, %s", errno, strerror(errno));
		return true;
	}

	bool Poll::setSocketPollin(SOCKET s, bool value) {
		struct epoll_event ee;
		ee.events = value ? (EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR) : (EPOLLET | EPOLLOUT | EPOLLERR);
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		/* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
		int rc = epoll_ctl(this->_epfd, EPOLL_CTL_MOD, s, &ee);
		CHECK_RETURN(rc == 0, false, "epoll_mod error: %d, %s, socket: %d", errno, strerror(errno), s);
		return true;
	}
	
	bool Poll::setSocketPollout(SOCKET s, bool value) {
		struct epoll_event ee;
		ee.events = value ? (EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR) : (EPOLLET | EPOLLIN | EPOLLERR);
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		/* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
		int rc = epoll_ctl(this->_epfd, EPOLL_CTL_MOD, s, &ee);
		CHECK_RETURN(rc == 0, false, "epoll_mod error: %d, %s, socket: %d", errno, strerror(errno), s);
		return true;
	}

	void Poll::wakeup() {
		this->addSocket(this->_wakefd);
	}

	void Poll::run(int milliseconds) {
		/* -1 to block indefinitely, 0 to return immediately, even if no events are available. */
		int numevents = ::epoll_wait(this->_epfd, this->_events, NM_POLL_EVENT, milliseconds);
		if (this->_isstop) {
			return;
		}
		
		if (numevents < 0) {
			if (errno == EINTR) {
				return; // wake up by signal
			}
			CHECK_RETURN(false, void(0), "epoll wait error:%d, %s", errno, strerror(errno));
		}
		
		for (int i = 0; i < numevents; ++i) {
			struct epoll_event* ee = &this->_events[i];
			if (ee->events & EPOLLERR) {
				//
				// if the server-side does not process messages in time,
				// error happens: Connection reset by peer
				Error("fd: %d poll error: %d, %d,%s", ee->data.fd, ee->events, errno, strerror(errno));
				this->_easynet->socketError(ee->data.fd);
			}
			else if (ee->events & EPOLLHUP) {
				Error("fd: %d poll hup: %d, %d,%s", ee->data.fd, ee->events, errno, strerror(errno));
				this->_easynet->socketError(ee->data.fd);
			}
			else if (ee->events & EPOLLRDHUP) {
				Error("fd: %d poll error or rdhup: %d", ee->data.fd, ee->events);
				this->_easynet->socketError(ee->data.fd);
			}
			else {
				if (ee->data.fd == this->_wakefd) {
					this->removeSocket(this->_wakefd);
				}
				else {
					if (ee->events & EPOLLIN) {
						this->_easynet->socketRead(ee->data.fd);
					}
					if (ee->events & EPOLLOUT) {
						this->_easynet->socketWrite(ee->data.fd);
					}
				}
			}
		}
	}
}


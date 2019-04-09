/*
 * \file: LockfreeQueue.h
 * \brief: Created by hushouguo at 14:53:41 Mar 25 2019
 */
 
#ifndef __LOCKFREEQUEUE_H__
#define __LOCKFREEQUEUE_H__

#include <list>

namespace logger {	
	template <typename T> class LockfreeQueue {
		public:
			inline void push_back(T node);
			inline void push_front(T node);
			inline T pop_back();
			inline T pop_front();
			inline size_t size();
			inline bool empty();
			inline void traverse(std::function<void(T)> invoke);

		private:
			Spinlocker _locker;
			std::list<T> _list;
	};

	template <typename T>
	void LockfreeQueue<T>::traverse(std::function<void(T)> invoke) {
		this->_locker.lock();
		for (auto& i : this->_list) {
			invoke(i);
		}
		this->_locker.unlock();
	}

	template <typename T> void LockfreeQueue<T>::push_back(T node) {
		this->_locker.lock();
		this->_list.push_back(node);
		this->_locker.unlock();
	}

	template <typename T> void LockfreeQueue<T>::push_front(T node) {
		this->_locker.lock();
		this->_list.push_front(node);
		this->_locker.unlock();
	}

	template <typename T> T LockfreeQueue<T>::pop_back() {
		assert(this->empty() == false);
		this->_locker.lock();
		T node = this->_list.back();
		this->_list.pop_back();
		this->_locker.unlock();
		return node;
	}

	template <typename T> T LockfreeQueue<T>::pop_front() {
		assert(this->empty() == false);
		this->_locker.lock();
		T node = this->_list.front();
		this->_list.pop_front();
		this->_locker.unlock();
		return node;
	}

	template <typename T> size_t LockfreeQueue<T>::size() {
		return this->_list.size();
	}

	template <typename T> bool LockfreeQueue<T>::empty() {
		return this->_list.empty();
	}
}

#endif

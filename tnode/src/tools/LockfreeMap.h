/*
 * \file: LockfreeMap.h
 * \brief: Created by hushouguo at 14:53:41 Mar 25 2019
 */
 
#ifndef __LOCKFREEMAP_H__
#define __LOCKFREEMAP_H__

#include "tools/Spinlocker.h"

BEGIN_NAMESPACE_TNODE {
	template <typename KEY, typename VALUE> class LockfreeMap {
		public:
			inline bool insert(KEY key, VALUE value);
			inline bool eraseKey(KEY key);
			inline bool containKey(KEY key);
			inline VALUE find(KEY key);
			inline size_t size();
			inline bool empty();
			inline VALUE pop_front();
			inline void traverse(std::function<void(KEY, VALUE)> invoke);

		private:
			Spinlocker _locker;
			std::unordered_map<KEY, VALUE> _map;
	};

	template <typename KEY, typename VALUE>
	void LockfreeMap<KEY, VALUE>::traverse(std::function<void(KEY, VALUE)> invoke) {
		this->_locker.lock();
		for (auto& i : this->_map) {
			invoke(i.first, i.second);
		}
		this->_locker.unlock();
	}
	
	template <typename KEY, typename VALUE>
	bool LockfreeMap<KEY, VALUE>::insert(KEY key, VALUE value) {
		this->_locker.lock();
		bool retval = this->_map.insert(std::make_pair(key, value)).second;
		this->_locker.unlock();
		return retval;
	}
	
	template <typename KEY, typename VALUE>
	bool LockfreeMap<KEY, VALUE>::eraseKey(KEY key) {
		bool retval = false;
		this->_locker.lock();
		auto i = this->_map.find(key);
		if (i != this->_map.end()) {
			retval = true;
			this->_map.erase(i);
		}
		this->_locker.unlock();
		return retval;
	}
	
	template <typename KEY, typename VALUE>
	bool LockfreeMap<KEY, VALUE>::containKey(KEY key) {
		this->_locker.lock();
		bool retval = this->_map.find(key) != this->_map.end();
		this->_locker.unlock();
		return retval;
	}
	
	template <typename KEY, typename VALUE>
	VALUE LockfreeMap<KEY, VALUE>::find(KEY key) {
		VALUE value = typename std::unordered_map<KEY, VALUE>::value_type::second_type();
		this->_locker.lock();
		auto i = this->_map.find(key);
		if (i != this->_map.end()) {
			value = i->second;
		}
		this->_locker.unlock();
		return value;
	}

	template <typename KEY, typename VALUE>
	VALUE LockfreeMap<KEY, VALUE>::pop_front() {
		VALUE value = typename std::unordered_map<KEY, VALUE>::value_type::second_type();
		this->_locker.lock();
		auto i = this->_map.begin();
		if (i != this->_map.end()) {
			value = i->second;
			this->_map.erase(i);
		}
		this->_locker.unlock();
		return value;
	}
	
	template <typename KEY, typename VALUE>
	size_t LockfreeMap<KEY, VALUE>::size() {
		this->_locker.lock();
		size_t size = this->_map.size();
		this->_locker.unlock();
		return size;
	}
	
	template <typename KEY, typename VALUE>
	bool LockfreeMap<KEY, VALUE>::empty() {
		this->_locker.lock();
		bool retval = this->_map.empty();
		this->_locker.unlock();
		return retval;
	}
}

#endif

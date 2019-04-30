/*
 * \file: ThreadPool.cpp
 * \brief: Created by hushouguo at 11:09:47 Mar 29 2019
 */

#include "common.h"

BEGIN_NAMESPACE_SLAM {
	void ThreadPool::init(int n) {
		int threads = n;
		if (threads <= 0) {
			threads = std::thread::hardware_concurrency();
			if (threads < 2) {
				threads = 2;
			}
		}
		Trace << "spawn threads: " << threads;

		while (threads-- > 0) {
			this->_threads.push_back(std::thread([this]{
				while (!this->_stop.load(std::memory_order_acquire)) {
					task_type task;
					{
						std::unique_lock<std::mutex> locker(this->_mtx);
						this->_cond.wait(locker, [this]{ 
							return this->_stop.load(std::memory_order_acquire) || !this->_tasks.empty();
							});
						if (this->_stop.load(std::memory_order_acquire)) {
							return;
						}
						task = std::move(this->_tasks.front());
						this->_tasks.pop();
					}
					task();
				}
			}));				
		}	
	}

	INITIALIZE_INSTANCE(ThreadPool);
}

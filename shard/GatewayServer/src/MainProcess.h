/*
 * \file: MainProcess.h
 * \brief: Created by hushouguo at 17:25:09 May 05 2019
 */
 
#ifndef __MAINPROCESS_H__
#define __MAINPROCESS_H__

BEGIN_NAMESPACE_SLAM {
	class MainProcess {
		public:
			void run();
			void wakeup();

		private:
			std::mutex _mtx;
			std::condition_variable _cond;
	};
}

#define sMainProcess slam::Singleton<slam::MainProcess>::getInstance()

#endif

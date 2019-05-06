/*
 * \file: MainProcess.h
 * \brief: Created by hushouguo at 11:21:34 May 06 2019
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

/*
 * \file: CentralClient.h
 * \brief: Created by hushouguo at 13:43:31 Apr 30 2019
 */
 
#ifndef __CENTRALCLIENT_H__
#define __CENTRALCLIENT_H__

BEGIN_NAMESPACE_SLAM {
	class CentralClient {
		public:
			bool init();
			void stop();
		
		public:	
			void run();

		private:
			Easynet* _easynet = nullptr;
	};
}

#define sCentralClient slam::Singleton<slam::CentralClient>::getInstance()

#endif

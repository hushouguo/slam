/*
 * \file: GatewayService.h
 * \brief: Created by hushouguo at 13:26:25 May 06 2019
 */
 
#ifndef __GATEWAYSERVICE_H__
#define __GATEWAYSERVICE_H__

BEGIN_NAMESPACE_SLAM {
	class GatewayService {
		public:
			bool init(const char* address, int port, size_t number = 0);
			void stop();

		public:
			void run();

		private:
			std::vector<Easynet*> _easynets;

		private:
			void run(Easynet* easynet);
	};
}

#define sGatewayService slam::Singleton<slam::GatewayService>::getInstance()

#endif

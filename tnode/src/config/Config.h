/*
 * \file: Config.h
 * \brief: Created by hushouguo at 15:28:22 Mar 25 2019
 */
 
#ifndef __CONFIG_H__
#define __CONFIG_H__

//
// NOTE: some config items in here
//

//
// Special max support number of service
#define TNODE_SERVICE_MAX_NUMBER				128

BEGIN_NAMESPACE_TNODE {
	class Config : public Registry {
		public:
			//
			// Has the marking the system stopped 
			//
			bool halt = false;

			//
			// Has the marking the system run in the background
			//
			bool runasdaemon = false;

			//
			// Has the marking the system need to reload configure files or data files
			//
			bool reload = false;

			//
			// Record the system terminate reason
			//	0: Normal exit, > 0: Signal terminated, < 0: Error code
			//
			int terminate_reason = 0;

			//
			// Record configure filename
			//
			std::string confile = "conf/conf.xml";

			//
			// Has the marking the system enable guard process
			//
			bool guard = false;

			//
			// Special spawn number of SeriveProcess, 0 means auto match
			//
			int threads = 0;

			//
			// Record entryfile
			//
			std::string entryfile = "scripts/init.lua";
			
			inline void syshalt(int reason = 0) {
				if (!this->halt) {
					terminate_reason = reason;
					this->halt = true;
				}
			}
			bool init(int argc, char* argv[]);

		private:
			bool loadconf(const char* xmlfile);
	};
}

#define sConfig tnode::Singleton<tnode::Config>::getInstance()

#endif

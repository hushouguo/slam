/*
 * \file: ServiceManager.h
 * \brief: Created by hushouguo at 15:23:05 Mar 25 2019
 */
 
#ifndef __SERVICEMANAGER_H__
#define __SERVICEMANAGER_H__

BEGIN_NAMESPACE_TNODE {
	class ServiceManager {
		public:
			ServiceManager();
			
		public:
			bool init(const char* entryfile);
			void stop();
			void schedule();
			u64 getFirstTimerExpire();
			
		public:
			bool pushMessage(const void* netmsg);
			Service* newservice(const char* entryfile);
			Service* getService(u32 sid);
			Service* getService(lua_State* L);
			bool exitservice(u32 sid);

		private:
			int _autosid = 0, _entrysid = -1;
			Service* _services[TNODE_SERVICE_MAX_NUMBER];
	};
}

#define sServiceManager tnode::Singleton<tnode::ServiceManager>::getInstance()

#endif

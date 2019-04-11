/*
 * \file: ServiceManager.h
 * \brief: Created by hushouguo at 15:23:05 Mar 25 2019
 */
 
#ifndef __SERVICEMANAGER_H__
#define __SERVICEMANAGER_H__

BEGIN_NAMESPACE_TNODE {
	class ServiceManager {
		public:
			bool init(const char* entryfile);
			void stop();
			
		public:
			bool pushMessage(const void* netmsg);
			Service* newservice(const char* entryfile);
			inline Service* getService(u32 sid) {
				Service* service = nullptr;
				this->_locker.lock();
				auto i = this->_services.find(sid);
				if (i != this->_services.end()) {
					service = i->second;
				}
				this->_locker.unlock();
				return service;
			}
			inline Service* getService(lua_State* L) {
				u32 sid = luaT_getOwner(L);
				Service* service = this->getService(sid);
				assert(service);
				return service;
			}
			bool exitservice(u32 sid);

		public:
			void schedule();
			void schedule(Service* service);
			
		private:
			int _autoid =  0;
			int _initid = -1;
			Spinlocker _locker;
			std::unordered_map<u32, Service*> _services;
			inline bool insertService(u32 sid, Service* service) {
				this->_locker.lock();
				bool rc = this->_services.insert(std::make_pair(sid, service)).second;
				this->_locker.unlock();
				return rc;
			}
			inline void removeService(u32 sid) {
				this->_locker.lock();
				auto i = this->_services.find(sid);
				if (i != this->_services.end()) {
					this->_services.erase(i);
				}
				this->_locker.unlock();
			}
	};
}

#define sServiceManager tnode::Singleton<tnode::ServiceManager>::getInstance()

#endif

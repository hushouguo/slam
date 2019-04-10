/*
 * \file: Service.cpp
 * \brief: Created by hushouguo at 12:46:57 Mar 26 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "message/ServiceMessage.h"
#include "config/Config.h"
#include "lua/luaT.h"
#include "lua/MessageParser.h"
#include "time/Time.h"
#include "service/Service.h"

BEGIN_NAMESPACE_TNODE {
	bool Service::init(const char* entryfile) {
		this->_messageParser = MessageParserCreator::create();
		this->_L = luaT_newstate();
		luaT_setOwner(this->_L, this->id);
		this->_entryfile = entryfile;
		/* run entry script file */
		if (!luaT_execFile(this->_L, this->_entryfile.c_str())) { return false; }		
		return true;
	}

	void Service::stop() {
		if (!this->_isstop) {
			this->_isstop = true;
			SafeDelete(this->_messageParser);
			luaT_close(this->_L);
			while (!this->_msgQueue.empty()) {
				const void* netmsg = this->_msgQueue.pop_front();
				sNetworkManager.easynet()->releaseMessage(netmsg);
			}
		}
	}

	void Service::run() {
		while (!this->isstop() && !this->_msgQueue.empty()) {
			const void* netmsg = this->_msgQueue.pop_front();
			assert(netmsg);
			this->msgParser(netmsg);
			sNetworkManager.easynet()->releaseMessage(netmsg);
		}
	}

	//
	// rc msgParser(fd, entityid, msgid, o)
	bool Service::msgParser(const void* netmsg) {
		size_t len = 0;
		const void* payload = sNetworkManager.easynet()->getMessageContent(netmsg, &len);
		CHECK_RETURN(len >= sizeof(ServiceMessage), false, "illegal netmsg.len: %ld, ServiceMessage: %ld", len, sizeof(ServiceMessage));
		const ServiceMessage* msg = (const ServiceMessage*) payload;
		CHECK_RETURN(len >= sizeof(ServiceMessage), false, "illegal netmsg.len: %ld, msg->len: %ld", len, msg->len);
		SOCKET fd = sNetworkManager.easynet()->getMessageSocket(netmsg);
		assert(fd != -1);
		
		luaT_getGlobalFunction(this->luaState(), __FUNCTION__);
		CHECK_RETURN(lua_isfunction(this->luaState(), -1), false, "not found `%s` function", __FUNCTION__);	
		lua_pushinteger(this->luaState(), fd);
		lua_pushinteger(this->luaState(), msg->entityid);
		lua_pushinteger(this->luaState(), msg->msgid);		
		bool rc = this->messageParser()->decode(this->luaState(), msg->msgid, msg->payload, msg->len - sizeof(ServiceMessage));
		if (rc) {
			luaT_Value ret;
			rc = luaT_pcall(this->luaState(), 4, ret);
		}
		else {
			Error << "decode message: " << msg->msgid << " error";
		}
		luaT_cleanup(this->luaState());
		return rc;
	}

	//
	// sid dispatch(entityid, msgid)
	u32 Service::dispatch(u64 entityid, u32 msgid) {
		luaT_getGlobalFunction(this->luaState(), __FUNCTION__);
		CHECK_RETURN(lua_isfunction(this->luaState(), -1), ILLEGAL_SERVICE, "not found `%s` function", __FUNCTION__);		
		luaT_Value ret;
		CHECK_RETURN(
			luaT_callFunction(this->luaState(), entityid,msgid, ret),
			ILLEGAL_SERVICE, "call `sid %s(entityid, msgid)` failure", __FUNCTION__);
		luaT_cleanup(this->luaState());
		CHECK_RETURN(ret.isinteger(), ILLEGAL_SERVICE, "`%s` return error type: %d", __FUNCTION__, ret.type);
		return ret.value_integer;
	}

	void Service::pushMessage(const void* netmsg) {
		this->_msgQueue.push_back(netmsg);
	}

	bool Service::need_schedule() {
		return !this->isstop() && this->_msgQueue.empty() == false;
	}
	
	void Service::regtimer(u32 milliseconds, int ref) {
		CHECK_RETURN(milliseconds > 0, void(0), "timer interval MUST greater than 0");
		timer_struct* ts = new timer_struct();
		sTime.now();
		ts->last_check = sTime.milliseconds();
		ts->milliseconds = milliseconds;
		ts->type = timer_forever;
		ts->ref = ref;
		this->_timerQueue.push_back(ts);
	}
}


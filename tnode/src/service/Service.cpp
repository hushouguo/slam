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
	Service::Service(u32 id) : Runnable(id) {
	}

	bool Service::init(const char* entryfile) {
		SafeDelete(this->_messageParser);
		const char* proto_dir = sConfig.get("tnode.protocol", (const char*) nullptr);
		CHECK_RETURN(proto_dir, false, "lack `tnode.protocol`");
		this->_messageParser = MessageParserCreator::create();
		bool retval = this->_messageParser->loadmsg(proto_dir);
		CHECK_RETURN(retval, false, "load protocol: %s error", proto_dir);

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
		}
	}

	void Service::run() {
#if 0		
		while (!this->isstop()) {
			const ServiceMessage* msg = this->getMessage();
			if (!msg) {
				break;
			}
			this->msgParser(msg);
			release_message(msg);
		}
#endif			
	}

	//
	// rc msgParser(fd, entityid, msgid, o)
	bool Service::msgParser(const ServiceMessage* msg) {
#if 0		
		luaT_getGlobalFunction(this->luaState(), __FUNCTION__);
		CHECK_RETURN(lua_isfunction(this->luaState(), -1), false, "not found `%s` function", __FUNCTION__);	
		lua_pushinteger(this->luaState(), msg->fd);
		lua_pushinteger(this->luaState(), msg->rawmsg.entityid);
		lua_pushinteger(this->luaState(), msg->rawmsg.msgid);		
		bool rc = this->messageParser()->decode(this->luaState(), msg->rawmsg.msgid, msg->rawmsg.payload, msg->rawmsg.len - sizeof(Socketmessage));
		if (rc) {
			luaT_Value ret;
			rc = luaT_pcall(this->luaState(), 4, ret);
		}
		else {
			Error << "decode message: " << msg->rawmsg.msgid << " error";
		}
		luaT_cleanup(this->luaState());
		return rc;
#else
		return false;
#endif		
	}

	//
	// sid dispatch(entityid, msgid)
	u32 Service::dispatch(u64 entityid, u32 msgid) {
#if 0		
		luaT_getGlobalFunction(this->luaState(), __FUNCTION__);
		CHECK_RETURN(lua_isfunction(this->luaState(), -1), ILLEGAL_SERVICE, "not found `%s` function", __FUNCTION__);		
		luaT_Value ret;
		CHECK_RETURN(
			luaT_callFunction(this->luaState(), entityid,msgid, ret),
			ILLEGAL_SERVICE, "call `sid %s(entityid, msgid)` failure", __FUNCTION__);
		luaT_cleanup(this->luaState());
		CHECK_RETURN(ret.isinteger(), ILLEGAL_SERVICE, "`%s` return error type: %d", __FUNCTION__, ret.type);
		return ret.value_integer;
#else
		return 0;
#endif		
	}

	void Service::pushMessage(const ServiceMessage* msg) {
		this->_msgQueue.push_back(msg);
	}

	bool Service::need_schedule() {
		return !this->isstop() && this->_msgQueue.empty() == false;
	}
	
	const ServiceMessage* Service::getMessage() {
		return this->_msgQueue.empty() ? nullptr : this->_msgQueue.pop_front();
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


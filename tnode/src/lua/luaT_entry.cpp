/*
 * \file: luaT_entry.cpp
 * \brief: Created by hushouguo at 10:15:21 Apr 11 2019
 */

#include "tnode.h"
#include "tools/Singleton.h"
#include "message/ServiceMessage.h"
#include "lua/luaT.h"
#include "lua/MessageParser.h"
#include "lua/luaT_entry.h"
#include "net/NetworkManager.h"

BEGIN_NAMESPACE_TNODE {
	//
	// u32 function dispatch(entityid, msgid)
	u32 luaT_entry_dispatch(lua_State* L, const void* netmsg) {
		size_t len = 0;
		const void* payload = sNetworkManager.easynet()->getMessageContent(netmsg, &len);
		CHECK_RETURN(len >= sizeof(ServiceMessage), false, "illegal netmsg.len: %ld, ServiceMessage: %ld", len, sizeof(ServiceMessage));
		const ServiceMessage* msg = (const ServiceMessage*) payload;
		CHECK_RETURN(len >= sizeof(ServiceMessage), false, "illegal netmsg.len: %ld, msg->len: %ld", len, msg->len);

		luaT_getGlobalFunction(L, "dispatch");
		CHECK_RETURN(lua_isfunction(L, -1), ILLEGAL_SERVICE, "not found `dispatch` function");

		luaT_Value ret;
		CHECK_RETURN(
			luaT_callFunction(L, msg->entityid, msg->msgid, ret),
			ILLEGAL_SERVICE, "call `sid dispatch(entityid, msgid)` failure");
		luaT_cleanup(L);
		CHECK_RETURN(ret.isinteger(), ILLEGAL_SERVICE, "`dispatch` return error type: %d", ret.type);
		return ret.value_integer;		
	}

	//
	// void function msgParser(fd, entityid, msgid, o)
	bool luaT_entry_msgParser(lua_State* L, const void* netmsg, MessageParser* msgParser) {
		size_t len = 0;
		const void* payload = sNetworkManager.easynet()->getMessageContent(netmsg, &len);
		CHECK_RETURN(len >= sizeof(ServiceMessage), false, "illegal netmsg.len: %ld, ServiceMessage: %ld", len, sizeof(ServiceMessage));
		const ServiceMessage* msg = (const ServiceMessage*) payload;
		CHECK_RETURN(len >= sizeof(ServiceMessage), false, "illegal netmsg.len: %ld, msg->len: %ld", len, msg->len);
		SOCKET fd = sNetworkManager.easynet()->getMessageSocket(netmsg);
		assert(fd != -1);
		
		luaT_getGlobalFunction(L, "msgParser");
		CHECK_RETURN(lua_isfunction(L, -1), false, "not found `msgParser` function");

		lua_pushinteger(L, fd);
		lua_pushinteger(L, msg->entityid);
		lua_pushinteger(L, msg->msgid);		
		bool rc = msgParser->decode(L, msg->msgid, msg->payload, msg->len - sizeof(ServiceMessage));
		if (rc) {
			luaT_Value ret;
			rc = luaT_pcall(L, 4, ret);
		}
		else {
			Error << "decode message: " << msg->msgid << " error";
		}
		luaT_cleanup(L);
		return rc;
	}
	
	//
	// void (timerid, ctx)
	bool luaT_entry_timer_expire(lua_State* L, u32 timerid, int ref, const luaT_Value& ctx) {
		luaT_getRegistry(L, ref);
		CHECK_RETURN(lua_isfunction(L, -1), false, "not found `anonymouse` function: %d", ref);

		lua_pushinteger(L, timerid);
		luaT_pushvalue(L, ctx);

		luaT_Value ret;
		bool rc = luaT_pcall(L, 2, ret);
		luaT_cleanup(L);
		
		return rc;
	}
}

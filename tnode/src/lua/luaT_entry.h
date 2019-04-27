/*
 * \file: luaT_entry.h
 * \brief: Created by hushouguo at 10:08:35 Apr 11 2019
 */
 
#ifndef __LUAT_ENTRY_H__
#define __LUAT_ENTRY_H__

BEGIN_NAMESPACE_TNODE {
	//
	// bool init(sid)
	bool luaT_entry_init(lua_State* L, u32 sid);
	//
	// void destroy()
	void luaT_entry_destroy(lua_State* L);
	//
	// u32 dispatch(entityid, msgid)
	u32 luaT_entry_dispatch(lua_State* L, const void* netmsg);
	//
	// void msgParser(fd, entityid, msgid, o)
	bool luaT_entry_msgParser(lua_State* L, const void* netmsg, MessageParser* msgParser);
	//
	// void hotfix_begin()
	void luaT_entry_hotfix_begin(lua_State* L);
	//
	// void hotfix_end()
	void luaT_entry_hotfix_end(lua_State* L);
	
	//
	// void timer()
	bool luaT_entry_timer_expire(lua_State* L, u32 timerid, int ref, const luaT_Value& ctx);
}

#endif

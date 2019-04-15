/*
 * \file: luaT_entry.h
 * \brief: Created by hushouguo at 10:08:35 Apr 11 2019
 */
 
#ifndef __LUAT_ENTRY_H__
#define __LUAT_ENTRY_H__

BEGIN_NAMESPACE_TNODE {
	//
	// u32 dispatch(entityid, msgid)
	u32 luaT_entry_dispatch(lua_State* L, const void* netmsg);
	//
	// void msgParser(fd, entityid, msgid, o)
	bool luaT_entry_msgParser(lua_State* L, const void* netmsg, luaT_message_parser* msgParser);
	
	//
	// void timer()
	bool luaT_entry_timer_expire(lua_State* L, u32 timerid, int ref, const luaT_Value& ctx);
}

#endif

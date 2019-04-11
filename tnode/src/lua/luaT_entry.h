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
	bool luaT_entry_msgParser(lua_State* L, const void* netmsg, MessageParser* msgParser);
}

#endif

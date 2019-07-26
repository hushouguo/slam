/*
 * \file: slam_event.c
 * \brief: Created by hushouguo at 15:48:06 Jul 23 2019
 */

#include "slam.h"

enum {
	EVENT_START = 1, 
	EVENT_STOP, 
	EVENT_MESSAGE, 
	EVENT_TIMER, 
	EVENT_CONNECTION_ESTABLISH, 
	EVENT_CONNECTION_LOST	
};

#define SLAM_LUA_EVENT_FUNC		"event_handle"

/* 
 * prototype: function event_handle(sessionid, event)
 *	event start: 
 *			{ 
 *				id = ?, 
 *				content = {
 *				}
 *			}
 *	event stop: 
 *			{ 
 *				id = ?, 
 *				content = {
 *				}
 *			}
 *	event establish connection: 
 *			{ 
 *				id = ?, 
 *				content = { 
 *					fd = ? 
 *				} 
 *			}
 *	event lost connection: 
 *			{ 
 *				id = ?, 
 *				content = { 
 *					fd = ? 
 *				} 
 *			}
 *	event message: 
 *			{
 *				id = ?,
 *				content = {
 *					fd = ?,
 *					entityid = ?,
 *					msg = {id = ?, ...}
 *				}
 *			}
 *	event timer: 
 *			{
 *				id = ?,
 *				content = {
 *					timerid = ?,
 *					context = ?
 *				}
 *			}
 */

bool slam_lua_event_start(slam_runnable_t* runnable) {
	bool rc;
	lua_State* L = slam_runnable_luastate(runnable);
	lua_getglobal(L, SLAM_LUA_EVENT_FUNC);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found `%s` func", SLAM_LUA_EVENT_FUNC);
	lua_pushinteger(L, (lua_Integer)(runnable)); /* sessionid */
	lua_newtable(L); /* event */
		lua_pushstring(L, "id"); /* key: event.id */
		lua_pushinteger(L, EVENT_START); /* value: event.id */
		lua_settable(L, -3);
	rc = slam_lua_pcall(L, 2, nullptr);
	slam_lua_cleanup(L);
	return rc;
}

bool slam_lua_event_stop(slam_runnable_t* runnable) {
	bool rc;
	lua_State* L = slam_runnable_luastate(runnable);
	lua_getglobal(L, SLAM_LUA_EVENT_FUNC);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found `%s` func", SLAM_LUA_EVENT_FUNC);
	lua_pushinteger(L, (lua_Integer)(runnable)); /* sessionid */
	lua_newtable(L); /* event */
		lua_pushstring(L, "id"); /* key: event.id */
		lua_pushinteger(L, EVENT_STOP); /* value: event.id */
		lua_settable(L, -3);
	rc = slam_lua_pcall(L, 2, nullptr);
	slam_lua_cleanup(L);
	return rc;
}

bool slam_lua_event_establish_connection(slam_runnable_t* runnable, SOCKET fd) {
	bool rc;
	lua_State* L = slam_runnable_luastate(runnable);
	lua_getglobal(L, SLAM_LUA_EVENT_FUNC);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found `%s` func", SLAM_LUA_EVENT_FUNC);
	lua_pushinteger(L, (lua_Integer)(runnable)); /* sessionid */
	lua_newtable(L); /* event */
		lua_pushstring(L, "id"); /* key: event.id */
		lua_pushinteger(L, EVENT_CONNECTION_ESTABLISH); /* value: event.id */
		lua_settable(L, -3);
		lua_pushstring(L, "fd"); /* key: event.id */
		lua_pushinteger(L, fd); /* value: event.id */
		lua_settable(L, -3);
	rc = slam_lua_pcall(L, 2, nullptr);
	slam_lua_cleanup(L);
	return rc;
}

bool slam_lua_event_lost_connection(slam_runnable_t* runnable, SOCKET fd) {
	bool rc;
	lua_State* L = slam_runnable_luastate(runnable);
	lua_getglobal(L, SLAM_LUA_EVENT_FUNC);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found `%s` func", SLAM_LUA_EVENT_FUNC);
	lua_pushinteger(L, (lua_Integer)(runnable)); /* sessionid */
	lua_newtable(L); /* event */
		lua_pushstring(L, "id"); /* key: event.id */
		lua_pushinteger(L, EVENT_CONNECTION_LOST); /* value: event.id */
		lua_settable(L, -3);
		lua_pushstring(L, "fd"); /* key: event.id */
		lua_pushinteger(L, fd); /* value: event.id */
		lua_settable(L, -3);
	rc = slam_lua_pcall(L, 2, nullptr);
	slam_lua_cleanup(L);
	return rc;
}


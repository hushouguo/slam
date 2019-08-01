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

// function slam_lua_event_start()
bool slam_lua_event_start(slam_runnable_t* runnable) {
	slam_lua_t* lua = slam_runnable_lua(runnable);
	lua_State* L = slam_luastate(lua);
	lua_getglobal(L, __FUNCTION__);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	return slam_lua_pcall(lua, 0, nullptr);
}

// function slam_lua_event_stop()
bool slam_lua_event_stop(slam_runnable_t* runnable) {
	slam_lua_t* lua = slam_runnable_lua(runnable);
	lua_State* L = slam_luastate(lua);
	lua_getglobal(L, __FUNCTION__);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	return slam_lua_pcall(lua, 0, nullptr);
}

// function slam_lua_event_establish_connection(fd)
bool slam_lua_event_establish_connection(slam_runnable_t* runnable, SOCKET fd) {
	slam_lua_t* lua = slam_runnable_lua(runnable);
	lua_State* L = slam_luastate(lua);
	lua_getglobal(L, __FUNCTION__);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	lua_pushinteger(L, fd);
	return slam_lua_pcall(lua, 1, nullptr);
}

// function slam_lua_event_lost_connection(fd)
bool slam_lua_event_lost_connection(slam_runnable_t* runnable, SOCKET fd) {
	slam_lua_t* lua = slam_runnable_lua(runnable);
	lua_State* L = slam_luastate(lua);
	lua_getglobal(L, __FUNCTION__);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	lua_pushinteger(L, fd);
	return slam_lua_pcall(lua, 1, nullptr);
}

// function slam_lua_event_message(fd, msgid, t)
bool slam_lua_event_message(slam_runnable_t* runnable, SOCKET fd, msgid_t msgid) {
	slam_lua_t* lua = slam_runnable_lua(runnable);
	lua_State* L = slam_luastate(lua);
	CHECK_RETURN(lua_istable(L, -1), false, "not found table in stack top");
	int t_index = lua_gettop(L);	// L: ... t ...
	lua_getglobal(L, __FUNCTION__); // L: ... t, func ...
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	lua_pushinteger(L, fd); 		// L: ... t, func, fd ...
	lua_pushinteger(L, msgid); 		// L: ... t, func, fd, msgid ...
	lua_pushvalue(L, t_index); 		// L: ... t, func, fd, msgid, t ...
	return slam_lua_pcall(lua, 3, nullptr);
}

// function (timerid, ctx)
bool slam_lua_event_timer(slam_runnable_t* runnable, slam_timer_t* timer) {
	slam_lua_t* lua = slam_runnable_lua(runnable);
	lua_State* L = slam_luastate(lua);
	int ref = slam_timer_ref(timer);
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    CHECK_RETURN(lua_isfunction(L, -1), false, "not found timer.`anonymouse` function on ref: %d", ref);
    lua_pushinteger(L, slam_timer_id(timer));
	slam_lua_pushvalue(lua, slam_timer_ctx(timer));
	bool rc = slam_lua_pcall(lua, 2, nullptr);
	slam_lua_cleanup(lua);
	return rc;
}


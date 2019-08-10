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
	lua_State* L = runnable->lua->L;
	lua_getglobal(L, __FUNCTION__);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	return slam_lua_pcall(runnable->lua, 0, nullptr);
}

// function slam_lua_event_stop()
bool slam_lua_event_stop(slam_runnable_t* runnable) {
	lua_State* L = runnable->lua->L;
	lua_getglobal(L, __FUNCTION__);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	return slam_lua_pcall(runnable->lua, 0, nullptr);
}

// function slam_lua_event_establish_connection(fd)
bool slam_lua_event_establish_connection(slam_runnable_t* runnable, SOCKET fd) {
	lua_State* L = runnable->lua->L;
	lua_getglobal(L, __FUNCTION__);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	lua_pushinteger(L, fd);
	return slam_lua_pcall(runnable->lua, 1, nullptr);
}

// function slam_lua_event_lost_connection(fd)
bool slam_lua_event_lost_connection(slam_runnable_t* runnable, SOCKET fd) {
	lua_State* L = runnable->lua->L;
	lua_getglobal(L, __FUNCTION__);
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", __FUNCTION__);
	lua_pushinteger(L, fd);
	return slam_lua_pcall(runnable->lua, 1, nullptr);
}

// function (fd, msgid, t)
bool slam_lua_event_message(slam_runnable_t* runnable, SOCKET fd, msgid_t msgid) {
	const char* ref_name = slam_protocol_find_name(runnable->protocol, msgid);
	CHECK_RETURN(ref_name != nullptr, false, "msgid: %d not reg", msgid);
	
	lua_State* L = runnable->lua->L;
	CHECK_RETURN(lua_istable(L, -1), false, "not found table in stack top");
	int t_index = lua_gettop(L);	    // L: ... t ...

	lua_pushstring(L, ref_name);
	lua_gettable(L, LUA_REGISTRYINDEX); // L: ... t, func ...	
	CHECK_RETURN(lua_isfunction(L, -1), false, "not found func: %s", ref_name);
	
	lua_pushinteger(L, fd); 		    // L: ... t, func, fd ...
	lua_pushinteger(L, msgid); 		    // L: ... t, func, fd, msgid ...
	lua_pushvalue(L, t_index); 		    // L: ... t, func, fd, msgid, t ...
	return slam_lua_pcall(runnable->lua, 3, nullptr);
}

// function (ref, ctx)
bool slam_lua_event_timer(slam_runnable_t* runnable, slam_timer_t* timer) {
	lua_State* L = runnable->lua->L;
	slam_lua_get_value_by_ref(runnable->lua, timer->ref);
    CHECK_RETURN(lua_isfunction(L, -1), false, "not found timer.`anonymouse` function on ref: %d", timer->ref);
    lua_pushinteger(L, timer->ref);
	slam_lua_pushvalue(runnable->lua, timer->ctx);
	bool rc = slam_lua_pcall(runnable->lua, 2, nullptr);
	slam_lua_cleanup(runnable->lua);
	return rc;
}


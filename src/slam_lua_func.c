/*
 * \file: slam_lua_func.c
 * \brief: Created by hushouguo at 10:02:17 Jul 30 2019
 */

#include "slam.h"

//
// fd newserver(address, port)
static int cc_newserver(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 2, 0, "`%s` lack args: %d", __FUNCTION__, args);
	CHECK_RETURN(lua_isstring(L, 1), 0, "[%s]", lua_typename(L, lua_type(L, 1)));
	CHECK_RETURN(lua_isnumber(L, 2), 0, "[%s]", lua_typename(L, lua_type(L, 2)));
	const char* address = lua_tostring(L, 1);
	int port = lua_tointeger(L, 2);
	SOCKET fd = slam_io_newserver(__slam_main->io, address, port);
	lua_pushinteger(L, fd);
	return 1;
}

//
// fd newclient(address, port)
static int cc_newclient(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 2, 0, "`%s` lack args: %d", __FUNCTION__, args);
	CHECK_RETURN(lua_isstring(L, 1), 0, "[%s]", lua_typename(L, lua_type(L, 1)));
	CHECK_RETURN(lua_isnumber(L, 2), 0, "[%s]", lua_typename(L, lua_type(L, 2)));
	const char* address = lua_tostring(L, 1);
	int port = lua_tointeger(L, 2);
	SOCKET fd = slam_io_newclient(__slam_main->io, address, port, SOCKET_CONNECT_TIMEOUT);
	lua_pushinteger(L, fd);
	return 1;
}

//
// bool response(fd, msgid, o)
static int cc_response(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, 1), 0, "[%s]", lua_typename(L, lua_type(L, 1)));
	CHECK_RETURN(lua_isnumber(L, 2), 0, "[%s]", lua_typename(L, lua_type(L, 2)));
	CHECK_RETURN(lua_istable(L, 3), 0, "[%s]", lua_typename(L, lua_type(L, 3)));	
	SOCKET fd = lua_tointeger(L, 1);
	msgid_t msgid = lua_tointeger(L, 2);
	log_trace("func: %s", __FUNCTION__);
	slam_message_t* message = slam_protocol_encode(__slam_main->runnable, fd, msgid);
	if (message) {
	    slam_io_response(__slam_main->io, message);
	}
	else {
		log_error("protocol encode error");
	}
	lua_pushboolean(L, message != nullptr);
	return 1;
}

//
// void closesocket(fd)
static int cc_closesocket(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, 1), 0, "[%s]", lua_typename(L, lua_type(L, 1)));
	SOCKET fd = lua_tointeger(L, 1);
	slam_io_closesocket(__slam_main->io, fd);
	return 0;
}

//
// bool loadmsg(filename) // filename also is a directory
static int cc_loadmsg(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isstring(L, 1), 0, "[%s]", lua_typename(L, lua_type(L, 1)));
	const char* filename = lua_tostring(L, 1);
	bool rc = slam_protocol_load_descriptor(__slam_main->runnable->protocol, filename);
	lua_pushboolean(L, rc);
	return 1;
}

//
// bool bindmsg(msgid, typename, function(fd, msgid, t) end)
static int cc_bindmsg(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, 1), 0, "[%s]", lua_typename(L, lua_type(L, 1)));
	CHECK_RETURN(lua_isstring(L, 2), 0, "[%s]", lua_typename(L, lua_type(L, 2)));
	CHECK_RETURN(lua_isfunction(L, 3), 0, "[%s]", lua_typename(L, lua_type(L, 3)));
	
	msgid_t msgid = lua_tointeger(L, 1);
	const char* typename = lua_tostring(L, 2);
	const char* ref_name = slam_protocol_reg_message(__slam_main->runnable->protocol, msgid, typename);
	if (ref_name != nullptr) {
	    lua_pushstring(L, ref_name);    // key: ref_name
	    lua_pushvalue(L, 3);  // value: lua function
	    lua_settable(L, LUA_REGISTRYINDEX);
	}
		
	lua_pushboolean(L, ref_name != nullptr);
	return 1;
}

//
// int newtimer(milliseconds, forever, ctx, function(id, ctx) end)
static int cc_newtimer(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, 1), 0, "[%s]", lua_typename(L, lua_type(L, 1)));
	CHECK_RETURN(lua_isboolean(L, 2), 0, "[%s]", lua_typename(L, lua_type(L, 2)));
	
	lua_Integer milliseconds = lua_tointeger(L, 1);
	bool forever = lua_toboolean(L, 2);

	slam_lua_value_t* ctx = slam_lua_value_new();
	if (lua_isboolean(L, 3)) {
		slam_lua_value_set_boolean(ctx, lua_toboolean(L, 3));
	}
	else if (lua_isnumber(L, 3)) {
		lua_Number value = lua_tonumber(L, 3);
		if (slam_is_integer(value)) {
			slam_lua_value_set_integer(ctx, lua_tointeger(L, 3));
		}
		else {
			slam_lua_value_set_number(ctx, lua_tonumber(L, 3));
		}
	}
	else if (lua_isstring(L, 3)) {
		slam_lua_value_set_string(ctx, lua_tostring(L, 3));
	}
	else if (lua_isnil(L, 3)) {
		slam_lua_value_set_nil(ctx);
	}
	else {
		log_error("not support ctx type: %s", lua_typename(L, lua_type(L, 3)));
	}
	
	CHECK_RETURN(lua_isfunction(L, 4), 0, "[%s]", lua_typename(L, lua_type(L, 4)));
	
	lua_pushvalue(L, 4);
	int ref = slam_lua_ref(__slam_main->runnable->lua);
	slam_timer_t* timer_node = slam_runnable_add_timer(__slam_main->runnable, milliseconds, forever, ref, ctx);
	
	lua_pushinteger(L, timer_node ? timer_node->ref : 0);
	return 1;
}

//
// void remove_timer(int ref)
static int cc_remove_timer(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, 1), 0, "[%s]", lua_typename(L, lua_type(L, 1)));
	lua_Integer ref = lua_tointeger(L, 1);
	slam_runnable_remove_timer(__slam_main->runnable, ref);
	return 0;
}

void slam_lua_reg_std_func(slam_lua_t* lua) {
	lua_State* L = lua->L;

	slam_lua_begin_namespace(lua, SLAM_LUA_REGISTER_NAMESPACE);

	/* proto */
	LUA_REGISTER(L, "loadmsg", cc_loadmsg); /* bool loadmsg(filename) */
	LUA_REGISTER(L, "bindmsg", cc_bindmsg); /* bool bindmsg(msgid, typename, function(fd, msgid, t) end) */
	
	/* network */
	LUA_REGISTER(L, "newserver", cc_newserver); /* fd newserver(address, port) */
	LUA_REGISTER(L, "newclient", cc_newclient); /* fd newclient(address, port) */
	LUA_REGISTER(L, "response", cc_response); 	/* bool response(fd, msgid, o) */
	LUA_REGISTER(L, "closesocket", cc_closesocket);	/* void closesocket(fd) */

	/* timer */
	LUA_REGISTER(L, "newtimer", cc_newtimer);/* int newtimer(milliseconds, forever, ctx, function(id, ctx) end) */
	LUA_REGISTER(L, "removetimer", cc_remove_timer); /* void remove_timer(int) */
	
	slam_lua_end_namespace(lua);
}


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
	CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
	CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
	const char* address = lua_tostring(L, -args);
	int port = lua_tointeger(L, -(args - 1));
	slam_socket_t* socket = slam_runnable_newserver(slam_main()->runnable, address, port);	
	lua_pushinteger(L, socket ? socket->fd : INVALID_SOCKET);
	return 1;
}

//
// fd newclient(address, port)
static int cc_newclient(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 2, 0, "`%s` lack args: %d", __FUNCTION__, args);
	CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
	CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
	const char* address = lua_tostring(L, -args);
	int port = lua_tointeger(L, -(args - 1));
	slam_socket_t* socket = slam_runnable_newclient(slam_main()->runnable, address, port, SOCKET_CONNECT_TIMEOUT);
	lua_pushinteger(L, socket ? socket->fd : INVALID_SOCKET);
	return 1;
}

//
// bool response(fd, msgid, o)
static int cc_response(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
	CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
	CHECK_RETURN(lua_istable(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
	
	SOCKET fd = lua_tointeger(L, -args);
	msgid_t msgid = lua_tointeger(L, -(args - 1));
	slam_socket_t* socket = slam_runnable_socket(slam_main()->runnable, fd);
	CHECK_RETURN(socket, 0, "not found socket: %d", fd);

	bool rc = slam_protocol_encode(slam_main()->runnable, msgid, socket);
	CHECK_RETURN(rc, 0, "protocol encode error");

	slam_runnable_write_socket(slam_main()->runnable, fd);

	lua_pushboolean(L, true);
	return 1;
}

//
// void closesocket(fd)
static int cc_closesocket(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
	SOCKET fd = lua_tointeger(L, -args);
	slam_runnable_remove_socket(slam_main()->runnable, fd);
	return 0;
}

//
// bool loadmsg(filename) // filename also is a directory
static int cc_loadmsg(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
	const char* filename = lua_tostring(L, -args);
	bool rc = slam_protocol_load_descriptor(slam_main()->runnable, filename);
	lua_pushboolean(L, rc);
	return 1;
}

//
// bool regmsg(msgid, name)
static int cc_regmsg(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
	CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
	msgid_t msgid = lua_tointeger(L, -args);
	const char* name = lua_tostring(L, -(args - 1));
	bool rc = slam_protocol_reg_message(slam_main()->runnable, msgid, name);
	lua_pushboolean(L, rc);
	return 1;
}

//
// u32 newtimer(milliseconds, forever, ctx, function(id, ctx) end)
static int cc_newtimer(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
	CHECK_RETURN(lua_isboolean(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
	
	lua_Integer milliseconds = lua_tointeger(L, -args);
	bool forever = lua_toboolean(L, -(args - 1));

	slam_lua_value_t* ctx = slam_lua_value_new();
	if (lua_isboolean(L, -(args - 2))) {
		slam_lua_value_set_boolean(ctx, lua_toboolean(L, -(args - 2)));
	}
	else if (lua_isnumber(L, -(args - 2))) {
		lua_Number value = lua_tonumber(L, -(args - 2));
		if (slam_is_integer(value)) {
			slam_lua_value_set_integer(ctx, lua_tointeger(L, -(args - 2)));
		}
		else {
			slam_lua_value_set_number(ctx, lua_tonumber(L, -(args - 2)));
		}
	}
	else if (lua_isstring(L, -(args - 2))) {
		slam_lua_value_set_string(ctx, lua_tostring(L, -(args - 2)));
	}
	else if (lua_isnil(L, -(args - 2))) {
		slam_lua_value_set_nil(ctx);
	}
	else {
		Error("not support ctx type: %s", lua_typename(L, lua_type(L, -(args - 2))));
	}
	
	CHECK_RETURN(lua_isfunction(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
	
	lua_pushvalue(L, -(args - 3));
	int ref = luaL_ref(L, LUA_REGISTRYINDEX); // set ref to registry table

	slam_timer_t* timer_node = slam_runnable_add_timer(slam_main()->runnable, milliseconds, forever, ref, ctx);
	
	lua_pushinteger(L, timer_node ? slam_timer_id(timer_node) : 0);
	return 1;
}

//
// void remove_timer(u32 timerid)
static int cc_remove_timer(lua_State* L) {
	int args = lua_gettop(L);
	CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
	CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
	lua_Integer timerid = lua_tointeger(L, -args);
	slam_runnable_remove_timer(slam_main()->runnable, timerid);
	return 0;
}

void slam_lua_reg_func(lua_State* L) {

	/* proto */
	LUA_REGISTER(L, "loadmsg", cc_loadmsg); /* bool loadmsg(filename) */
	LUA_REGISTER(L, "regmsg", cc_regmsg); /* bool regmsg(msgid, name) */
	
	/* network */
	LUA_REGISTER(L, "newserver", cc_newserver); /* fd newserver(address, port) */
	LUA_REGISTER(L, "newclient", cc_newclient); /* fd newclient(address, port) */
	LUA_REGISTER(L, "response", cc_response); 	/* bool response(fd, msgid, o) */
	LUA_REGISTER(L, "closesocket", cc_closesocket);	/* void closesocket(fd) */

	/* timer */
	LUA_REGISTER(L, "newtimer", cc_newtimer);/* u32 newtimer(milliseconds, forever, ctx, function(id, ctx) end) */
	LUA_REGISTER(L, "removetimer", cc_remove_timer); /* void remove_timer(u32) */
	
}


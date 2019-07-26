/*
 * \file: luaT.c
 * \brief: Created by hushouguo at 17:14:55 Jul 25 2019
 */

#include "slam.h"

lua_State* slam_lua_newstate(size_t stack_size) {
	lua_State* L = luaL_newstate(); 	
	luaL_openlibs(L);
	lua_checkstack(L, stack_size);
	/* TODO: error handler & allocator */
	return L;
}

void slam_lua_closestate(lua_State* L) {
	lua_close(L);
}

void slam_lua_showversion(lua_State* L) {
#if SLAM_LUA_USE_LUAJIT
	Debug("JIT: %s -- %s", LUAJIT_VERSION, LUAJIT_COPYRIGHT);		
	lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_getfield(L, -1, "jit");  /* Get jit.* module table. */
	lua_remove(L, -2);
	lua_getfield(L, -1, "status");
	lua_remove(L, -2);
	int n = lua_gettop(L);
	lua_call(L, 0, LUA_MULTRET);
	Debug("JIT: %s", lua_toboolean(L, n) ? "ON" : "OFF");
#else
	Alarm("JIT: not found");
#endif
}

const char* slam_lua_tostring(lua_State* L, int idx) {
	if (lua_istable(L, idx)) { return "[table]"; }
	else if (lua_isnone(L, idx)) { return "[none]"; }
	else if (lua_isnil(L, idx)) { return "[nil]"; }
	else if (lua_isboolean(L, idx)) {
		return lua_toboolean(L, idx) != 0 ? "[true]" : "[false]";
	}
	else if (lua_isfunction(L, idx)) { return "[function]"; }
	else if (lua_isuserdata(L, idx)) { return "[userdata]"; }
	else if (lua_islightuserdata(L, idx)) { return "[lightuserdata]"; }
	else if (lua_isthread(L, idx)) { return "[thread]"; }
	else {
		return lua_tostring(L, idx);
	}
	return lua_typename(L, lua_type(L, idx));
}

bool slam_lua_pcall(lua_State* L, int args, slam_lua_value_t* lvalue) {
	int error, traceback = 0;
	int func_idx = -(args + 1);
	
	if (!lua_isfunction(L, func_idx)) {
		Error("value at stack [%d] is not function\n", func_idx);
		lua_pop(L, args + 1);/* remove function and args */
		return false;
	}
	
	lua_getglobal(L, "__G_TRACKBACK__");	/* L: ...func arg1 arg2 ... G */
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 1);						/* L: ...func arg1 arg2 ... */
	}
	else {
		lua_insert(L, func_idx - 1);		/* L: ... G func arg1 arg2 ... */
		traceback = func_idx - 1;
	}
	
	error = lua_pcall(L, args, 1, traceback);

	/* check error */
	if (error) {
		if (traceback == 0) {
			Error("%s\n", lua_tostring(L, -1));/* ... error */
			lua_pop(L, 1); /* remove error message from stack */
		}
		else {
			lua_pop(L, 2); /* remove __G_TRACKBACK__ and error message from stack */
		}
		return false;
	}
	
	/* get return value */
	if (lvalue != nullptr) {
		if (lua_isboolean(L, -1)) {
			slam_lua_value_set_boolean(lvalue, lua_toboolean(L, -1)); /* lua_Boolean: 0 or 1 */
		}
		else if (lua_isnumber(L, -1)) {
			lua_Number value = lua_tonumber(L, -1);
			if (slam_is_integer(value)) {
				slam_lua_value_set_integer(lvalue, lua_tointeger(L, -1));
			}
			else {
				slam_lua_value_set_number(lvalue, lua_tonumber(L, -1));
			}
		}
		else if (lua_isstring(L, -1)) {
			slam_lua_value_set_string(lvalue, lua_tostring(L, -1));
		}
		else {
			Error("Not support return type: %s", lua_typename(L, lua_type(L, -1)));
		}
	}
	
	lua_pop(L, 1); /* remove return value from stack, L: ... [G] */
	
	if (traceback) {
		lua_pop(L, 1); /* remove __G_TRACKBACK__ from stack, L: ... */
	}
	
	return true;
}

bool slam_lua_dostring(lua_State* L, const char* str) {
	luaL_loadstring(L, str);
	return slam_lua_pcall(L, 0, nullptr);
}

bool slam_lua_dofile(lua_State* L, const char* filename) {
	char buf[PATH_MAX], buffer[PATH_MAX], *suffix = nullptr;
	snprintf(buf, sizeof(buf), "%s", filename);
	suffix = strrchr(buf, '.');
	if (suffix && strcasecmp(suffix, ".lua") == 0) {
		*suffix = '\0';
	}
	snprintf(buffer, sizeof(buffer), "require \"%s\"", buf);
	return slam_lua_dostring(L, buffer);
}

void slam_lua_cleanup(lua_State* L) {
	lua_settop(L, 0);
}

void slam_lua_tracestack(lua_State* L) {
	int i, args = lua_gettop(L);
	Debug("lua tracestack: %d", args);
	for (i = 1; i <= args; ++i) {
		Debug("	[%s]  (%s)", slam_lua_tostring(L, i), lua_typename(L, lua_type(L, i)));
	}
}

void slam_lua_dump_table(lua_State* L, int idx, const char* prefix) {
	lua_pushnil(L);
	while(lua_next(L, idx)) {
		const char *key, *value;
		/* -2 : key, -1 : value */
		lua_pushvalue(L, -2);
		key = lua_tostring(L, -1);
		value = slam_lua_tostring(L, -2);
		lua_pop(L, 1);
	
		Debug("%s%15s: %s", prefix, key, value);
	
		if (lua_istable(L, -1) && strcasecmp(key, SLAM_LUA_REGISTER_NAMESPACE) == 0) {
			char buffer[960];
			snprintf(buffer, sizeof(buffer), "%s\t\t", prefix);
			Debug("%15s{", prefix);
			slam_lua_dump_table(L, lua_gettop(L), buffer);
			Debug("%15s}", prefix);
		}
	
		lua_pop(L, 1);/* removes 'value'; keeps 'key' for next iteration */
	}
}

void slam_lua_dump_root_table(lua_State* L) {
	lua_getglobal(L, "_G");
	Debug("dump root table");
	Debug("{");
	slam_lua_dump_table(L, lua_gettop(L), "\t");
	Debug("}");
	lua_pop(L, 1);/* remove `table` */
}

void slam_lua_dump_registry_table(lua_State* L) {
	lua_getregistry(L);
	Debug("dump registry table");
	Debug("{");
	slam_lua_dump_table(L, lua_gettop(L), "\t");
	Debug("}");
	lua_pop(L, 1);/* remove `table` */
}

void slam_lua_reg_namespace(lua_State* L, const char* ns) {
	lua_getglobal(L, "_G");
	lua_pushstring(L, ns);
	lua_newtable(L);
	lua_rawset(L, -3);
	lua_pop(L, 1);
}

void slam_lua_begin_namespace(lua_State* L, const char* ns) {
	lua_getglobal(L, "_G");
	lua_pushstring(L, ns);
	lua_rawget(L, -2);
}

void slam_lua_end_namespace(lua_State* L) {
	lua_pop(L, 2);
}

void slam_lua_reg_function(lua_State* L, const char* func, lua_CFunction routine) {
	lua_pushstring(L, func);
	lua_pushcfunction(L, routine);
	lua_rawset(L, -3);
}

void slam_lua_reg_global_function(lua_State* L, const char* func, lua_CFunction routine) {
	lua_getglobal(L, "_G");
	slam_lua_reg_function(L, func, routine);
	lua_pop(L, 1);
}

void slam_lua_reg_integer(lua_State* L, const char* key, int value) {
	lua_pushstring(L, key);
	lua_pushinteger(L, value);
	lua_settable(L, -3);
}

void slam_lua_reg_long(lua_State* L, const char* key, long value) {
	lua_pushstring(L, key);
	lua_pushnumber(L, value);
	lua_settable(L, -3);
}

void slam_lua_reg_double(lua_State* L, const char* key, double value) {
	lua_pushstring(L, key);
	lua_pushnumber(L, value);
	lua_settable(L, -3);
}

void slam_lua_reg_string(lua_State* L, const char* key, const char* value) {
	lua_pushstring(L, key);
	lua_pushstring(L, value);
	lua_settable(L, -3);
}

void slam_lua_reg_object(lua_State* L, const char* key, const void* value) {
	lua_pushstring(L, key);
	lua_pushlightuserdata(L, (void*) value);
	lua_rawset(L, -3);
}

void slam_lua_pushvalue(lua_State* L, slam_lua_value_t* lvalue) {
	if (slam_lua_value_is_nil(lvalue)) {
		lua_pushnil(L);
	}
	else if (slam_lua_value_is_integer(lvalue)) {
		lua_pushinteger(L, slam_lua_value_get_integer(lvalue));
	}
	else if (slam_lua_value_get_number(lvalue)) {
		lua_pushnumber(L, slam_lua_value_get_number(lvalue));
	}
	else if (slam_lua_value_get_boolean(lvalue)) {
		lua_pushboolean(L, slam_lua_value_get_boolean(lvalue));
	}
	else if (slam_lua_value_is_string(lvalue)) {
		lua_pushstring(L, slam_lua_value_get_string(lvalue));
	}
	else {
		Error("illegal lvalue->type: %d", slam_lua_value_type(lvalue));
		lua_pushnil(L);
	}
}


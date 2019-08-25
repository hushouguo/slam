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

slam_lua_t* slam_lua_new(size_t stack_size) {
	slam_lua_t* lua = (slam_lua_t *) slam_malloc(sizeof(slam_lua_t));
	lua->L = slam_lua_newstate(stack_size);
	lua->entryfile = nullptr;
	lua->stack_size = stack_size;
	slam_lua_reg_namespace(lua, SLAM_LUA_REGISTER_NAMESPACE);
	slam_lua_reg_std_func(lua);
	//slam_lua_reg_db_func(lua);
	return lua;
}

void slam_lua_delete(slam_lua_t* lua) {
	slam_lua_closestate(lua->L);
	slam_free(lua->entryfile);
	slam_free(lua);
}


bool slam_lua_load_entryfile(slam_lua_t* lua, const char* entryfile) {
	bool rc = slam_lua_dofile(lua, entryfile);
	if (rc) {
		slam_free(lua->entryfile);
		lua->entryfile = strdup(entryfile);
	}
	return rc;
}

void slam_lua_showversion(slam_lua_t* lua) {
#if SLAM_LUA_USE_LUAJIT
	log_debug("JIT: %s -- %s", LUAJIT_VERSION, LUAJIT_COPYRIGHT);		
	lua_getfield(lua->L, LUA_REGISTRYINDEX, "_LOADED");
	lua_getfield(lua->L, -1, "jit");  /* Get jit.* module table. */
	lua_remove(lua->L, -2);
	lua_getfield(lua->L, -1, "status");
	lua_remove(lua->L, -2);
	int n = lua_gettop(lua->L);
	lua_call(lua->L, 0, LUA_MULTRET);
	log_debug("JIT: %s", lua_toboolean(lua->L, n) ? "ON" : "OFF");
#else
	Alarm("JIT: not found");
#endif
}

const char* slam_lua_tostring(slam_lua_t* lua, int idx) {
	if (lua_istable(lua->L, idx)) { return "[table]"; }
	else if (lua_isnone(lua->L, idx)) { return "[none]"; }
	else if (lua_isnil(lua->L, idx)) { return "[nil]"; }
	else if (lua_isboolean(lua->L, idx)) {
		return lua_toboolean(lua->L, idx) != 0 ? "[true]" : "[false]";
	}
	else if (lua_isfunction(lua->L, idx)) { return "[function]"; }
	else if (lua_isuserdata(lua->L, idx)) { return "[userdata]"; }
	else if (lua_islightuserdata(lua->L, idx)) { return "[lightuserdata]"; }
	else if (lua_isthread(lua->L, idx)) { return "[thread]"; }
	else {
		return lua_tostring(lua->L, idx);
	}
	return lua_typename(lua->L, lua_type(lua->L, idx));
}

const char* slam_L_tostring(lua_State* L, int idx) {
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

bool slam_lua_pcall(slam_lua_t* lua, int args, slam_lua_value_t* lvalue) {
	int error, traceback = 0;
	int func_idx = -(args + 1);
	
	if (!lua_isfunction(lua->L, func_idx)) {
		log_error("value at stack [%d] is not function\n", func_idx);
		lua_pop(lua->L, args + 1);/* remove function and args */
		return false;
	}
	
	lua_getglobal(lua->L, "__G_TRACKBACK__");	/* L: ...func arg1 arg2 ... G */
	if (!lua_isfunction(lua->L, -1)) {
		lua_pop(lua->L, 1);						/* L: ...func arg1 arg2 ... */
	}
	else {
		lua_insert(lua->L, func_idx - 1);		/* L: ... G func arg1 arg2 ... */
		traceback = func_idx - 1;
	}
	
	error = lua_pcall(lua->L, args, 1, traceback);

	/* check error */
	if (error) {
		if (traceback == 0) {
			log_error("%s\n", lua_tostring(lua->L, -1));/* ... error */
			lua_pop(lua->L, 1); /* remove error message from stack */
		}
		else {
			lua_pop(lua->L, 2); /* remove __G_TRACKBACK__ and error message from stack */
		}
		return false;
	}
	
	/* get return value */
	if (lvalue != nullptr) {
		if (lua_isboolean(lua->L, -1)) {
			slam_lua_value_set_boolean(lvalue, lua_toboolean(lua->L, -1)); /* lua_Boolean: 0 or 1 */
		}
		else if (lua_isnumber(lua->L, -1)) {
			lua_Number value = lua_tonumber(lua->L, -1);
			if (slam_is_integer(value)) {
				slam_lua_value_set_integer(lvalue, lua_tointeger(lua->L, -1));
			}
			else {
				slam_lua_value_set_number(lvalue, lua_tonumber(lua->L, -1));
			}
		}
		else if (lua_isstring(lua->L, -1)) {
			slam_lua_value_set_string(lvalue, lua_tostring(lua->L, -1));
		}
		else {
			log_error("Not support return type: %s", lua_typename(lua->L, lua_type(lua->L, -1)));
		}
	}
	
	lua_pop(lua->L, 1); /* remove return value from stack, L: ... [G] */
	
	if (traceback) {
		lua_pop(lua->L, 1); /* remove __G_TRACKBACK__ from stack, L: ... */
	}
	
	return true;
}

bool slam_lua_dostring(slam_lua_t* lua, const char* str) {
	luaL_loadstring(lua->L, str);
	return slam_lua_pcall(lua, 0, nullptr);
}

bool slam_lua_dofile(slam_lua_t* lua, const char* filename) {
	char buf[PATH_MAX], buffer[PATH_MAX], *suffix = nullptr;
	snprintf(buf, sizeof(buf), "%s", filename);
	suffix = strrchr(buf, '.');
	if (suffix && strcasecmp(suffix, ".lua") == 0) {
		*suffix = '\0';
	}
	snprintf(buffer, sizeof(buffer), "require \"%s\"", buf);
	return slam_lua_dostring(lua, buffer);
}

void slam_lua_cleanup(slam_lua_t* lua) {
	lua_settop(lua->L, 0);
}

void slam_lua_tracestack(slam_lua_t* lua) {
	int i, args = lua_gettop(lua->L);
	log_debug("lua tracestack: %d", args);
	for (i = 1; i <= args; ++i) {
		log_debug("	[%s]  (%s)", slam_lua_tostring(lua, i), lua_typename(lua->L, lua_type(lua->L, i)));
	}
}

void slam_lua_dump_table(lua_State* L, int idx, const char* prefix) {
	lua_pushnil(L);
	while(lua_next(L, idx)) {
		const char *key, *value;
		/* -2 : key, -1 : value */
		lua_pushvalue(L, -2);
		key = lua_tostring(L, -1);
		value = slam_L_tostring(L, -2);
		lua_pop(L, 1);
	
		log_debug("%s%15s: %s", prefix, key, value);
	
		if (lua_istable(L, -1) && strcasecmp(key, SLAM_LUA_REGISTER_NAMESPACE) == 0) {
			char buffer[960];
			snprintf(buffer, sizeof(buffer), "%s\t\t", prefix);
			log_debug("%15s{", prefix);
			slam_lua_dump_table(L, lua_gettop(L), buffer);
			log_debug("%15s}", prefix);
		}
	
		lua_pop(L, 1);/* removes 'value'; keeps 'key' for next iteration */
	}
}

void slam_lua_dump_root_table(lua_State* L) {
	lua_getglobal(L, "_G");
	log_debug("dump root table");
	log_debug("{");
	slam_lua_dump_table(L, lua_gettop(L), "\t");
	log_debug("}");
	lua_pop(L, 1);/* remove `table` */
}

void slam_lua_reg_namespace(slam_lua_t* lua, const char* ns) {
	lua_getglobal(lua->L, "_G");
	lua_pushstring(lua->L, ns);
	lua_newtable(lua->L);
	lua_rawset(lua->L, -3);
	lua_pop(lua->L, 1);
}

void slam_lua_begin_namespace(slam_lua_t* lua, const char* ns) {
	lua_getglobal(lua->L, "_G");
	lua_pushstring(lua->L, ns);
	lua_rawget(lua->L, -2);
}

void slam_lua_end_namespace(slam_lua_t* lua) {
	lua_pop(lua->L, 2);
}

void slam_lua_reg_function(slam_lua_t* lua, const char* func, lua_CFunction routine) {
	lua_pushstring(lua->L, func);
	lua_pushcfunction(lua->L, routine);
	lua_rawset(lua->L, -3);
}

void slam_lua_reg_global_function(slam_lua_t* lua, const char* func, lua_CFunction routine) {
	lua_getglobal(lua->L, "_G");
	slam_lua_reg_function(lua, func, routine);
	lua_pop(lua->L, 1);
}

void slam_lua_reg_integer(slam_lua_t* lua, const char* key, int value) {
	lua_pushstring(lua->L, key);
	lua_pushinteger(lua->L, value);
	lua_settable(lua->L, -3);
}

void slam_lua_reg_long(slam_lua_t* lua, const char* key, long value) {
	lua_pushstring(lua->L, key);
	lua_pushnumber(lua->L, value);
	lua_settable(lua->L, -3);
}

void slam_lua_reg_double(slam_lua_t* lua, const char* key, double value) {
	lua_pushstring(lua->L, key);
	lua_pushnumber(lua->L, value);
	lua_settable(lua->L, -3);
}

void slam_lua_reg_string(slam_lua_t* lua, const char* key, const char* value) {
	lua_pushstring(lua->L, key);
	lua_pushstring(lua->L, value);
	lua_settable(lua->L, -3);
}

void slam_lua_reg_object(slam_lua_t* lua, const char* key, const void* value) {
	lua_pushstring(lua->L, key);
	lua_pushlightuserdata(lua->L, (void*) value);
	lua_rawset(lua->L, -3);
}

void slam_lua_pushvalue(slam_lua_t* lua, slam_lua_value_t* lvalue) {
	if (lvalue == nullptr) {
		lua_pushnil(lua->L);
	}
	else if (slam_lua_value_is_nil(lvalue)) {
		lua_pushnil(lua->L);
	}
	else if (slam_lua_value_is_integer(lvalue)) {
		lua_pushinteger(lua->L, slam_lua_value_get_integer(lvalue));
	}
	else if (slam_lua_value_is_number(lvalue)) {
		lua_pushnumber(lua->L, slam_lua_value_get_number(lvalue));
	}
	else if (slam_lua_value_is_boolean(lvalue)) {
		lua_pushboolean(lua->L, slam_lua_value_get_boolean(lvalue));
	}
	else if (slam_lua_value_is_string(lvalue)) {
		lua_pushstring(lua->L, slam_lua_value_get_string(lvalue));
	}
	else {
		log_error("illegal lvalue->type: %d", slam_lua_value_type(lvalue));
		lua_pushnil(lua->L);
	}
}

void slam_lua_dump_registry_table(slam_lua_t* lua) {
	lua_getregistry(lua->L);
	log_debug("dump registry table");
	log_debug("{");
	slam_lua_dump_table(lua->L, lua_gettop(lua->L), "\t");
	log_debug("}");
	lua_pop(lua->L, 1);/* remove `table` */
}

// value at the top of stack, return key
int slam_lua_ref(slam_lua_t* lua) {
    return luaL_ref(lua->L, LUA_REGISTRYINDEX); // set value to registry table and return ref-key
}

// unref from registry
void slam_lua_unref(slam_lua_t* lua, int ref) {
    luaL_unref(lua->L, LUA_REGISTRYINDEX, ref); // clear this ref from registry
}

// return value to the top of stack
void slam_lua_get_value_by_ref(slam_lua_t* lua, int ref) {
    lua_rawgeti(lua->L, LUA_REGISTRYINDEX, ref);// get value from registry and put it the top of stack
}



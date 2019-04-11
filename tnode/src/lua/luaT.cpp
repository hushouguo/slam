/*
 * \file: luaT.cpp
 * \brief: Created by hushouguo at 15:00:47 Apr 04 2019
 */

#include "tnode.h"
#include "tools/Tools.h"
#include "lua/luaT.h"
#include "lua/luaT_reg_functions.h"

BEGIN_NAMESPACE_TNODE {
	lua_State* luaT_newstate(u32 stackSize) {
		lua_State* L = luaL_newstate();		
		luaL_openlibs(L);
		lua_checkstack(L, stackSize/*LUA_STACK_SIZE*/);
		//TODO: error handler & allocator 

		luaT_regNamespace(L, LUA_REGISTER_NAMESPACE);
		luaT_reg_functions(L);
		
		return L;
	}
	
	void luaT_close(lua_State* L) {
		lua_close(L);
	}

	void luaT_setOwner(lua_State* L, u32 sid) {
		lua_pushinteger(L, sid);
		lua_setglobal(L, LUA_REGISTER_SERVICE);
	}
	
	u32 luaT_getOwner(lua_State* L) {
		lua_getglobal(L, LUA_REGISTER_SERVICE);
		CHECK_RETURN(!lua_isnoneornil(L, -1), -1, "Not owner: %s", LUA_REGISTER_SERVICE);
		u32 sid = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return sid;
	}

	void luaT_showversion(lua_State* L) {
#if defined(USE_LUAJIT)
		Debug.cout("JIT: %s -- %s", LUAJIT_VERSION, LUAJIT_COPYRIGHT);		
		lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
		lua_getfield(L, -1, "jit");  /* Get jit.* module table. */
		lua_remove(L, -2);
		lua_getfield(L, -1, "status");
		lua_remove(L, -2);
		int n = lua_gettop(L);
		lua_call(L, 0, LUA_MULTRET);
		Debug.cout("JIT: %s", lua_toboolean(L, n) ? "ON" : "OFF");
#else
		Alarm.cout("JIT: not found");
#endif
	}
	
	const char* luaT_tostring(lua_State* L, int idx) {
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
	
	bool luaT_pcall(lua_State* L, int args, luaT_Value& ret) {
		int func_idx = -(args + 1);
		if (!lua_isfunction(L, func_idx)) {
			Error.cout("value at stack [%d] is not function\n", func_idx);
			lua_pop(L, args + 1);/* remove function and args */
			return false;
		}

		int traceback = 0;
		lua_getglobal(L, "__G_TRACKBACK__"); 	/* L: ...func arg1 arg2 ... G */
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 1);						/* L: ...func arg1 arg2 ... */
		}
		else {
			lua_insert(L, func_idx - 1);		/* L: ... G func arg1 arg2 ... */
			traceback = func_idx - 1;
		}

		int error = lua_pcall(L, args, 1, traceback);

		if (error) {
			if (traceback == 0) {
				Error.cout("%s\n", lua_tostring(L, -1));/* ... error */
				lua_pop(L, 1); /* remove error message from stack */
			}
			else {
				lua_pop(L, 2); /* remove __G_TRACKBACK__ and error message from stack */
			}
			return false;
		}

		// get return value
		if (lua_isboolean(L, -1)) {
			ret = lua_toboolean(L, -1) != 0;
		}
		else if (lua_isnumber(L, -1)) {
			lua_Number value = lua_tonumber(L, -1);
			if (isInteger(value)) {
				ret = lua_tointeger(L, -1);
			}
			else {
				ret = lua_tonumber(L, -1);
			}
		}
		else if (lua_isstring(L, -1)) {
			ret = lua_tostring(L, -1);
		}
		else {
			//Alarm << "Not support return type: " << lua_typename(L, lua_type(L, -1));
		}

		lua_pop(L, 1); /* remove return value from stack, L: ... [G] */
		if (traceback) {
			lua_pop(L, 1); /* remove __G_TRACKBACK__ from stack, L: ... */
		}
		return true;
	}

	// callFunction
	bool luaT_callFunction(lua_State* L, luaT_Value& ret) {
		return luaT_pcall(L, 0, ret);
	}
	
	void luaT_getRegistry(lua_State* L, int ref) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	}

	void luaT_getGlobalFunction(lua_State* L, const char* func) {
		lua_getglobal(L, func);
	}
	
	
	bool luaT_execString(lua_State* L, const char* s) {
		luaL_loadstring(L, s);
		luaT_Value ret;
		return luaT_pcall(L, 0, ret);
	}
	
	bool luaT_execFile(lua_State* L, const char* filename) {
		char buf[PATH_MAX];
		snprintf(buf, sizeof(buf), "%s", filename);
		char* suffix = strrchr(buf, '.');
		if (suffix && strcasecmp(suffix, ".lua") == 0) {
			*suffix = '\0';
		}
		char buffer[PATH_MAX];
		snprintf(buffer, sizeof(buffer), "require \"%s\"", buf);
		return luaT_execString(L, buffer);
	}
		
	void luaT_cleanup(lua_State* L) {
		lua_settop(L, 0);
	}

	void luaT_tracestack(lua_State* L) {
		int args = lua_gettop(L);
		Debug << "lua tracestack: " << args;
		for (int i = 1; i <= args; ++i) {
			Debug << "  [" << i << "]  " << luaT_tostring(L, i) << "(" << lua_typename(L, lua_type(L, i)) << ")";
		}
	}

	void luaT_dumpTable(lua_State* L, int idx, const char* prefix) {
		lua_pushnil(L);
		while(lua_next(L, idx)) {
			/* -2 : key, -1 : value */
			lua_pushvalue(L, -2);
			const char* key = lua_tostring(L, -1);
			const char* value = luaT_tostring(L, -2);
			lua_pop(L, 1);

			Debug.cout("%s%15s: %s", prefix, key, value);

			if (lua_istable(L, -1) && strcasecmp(key, LUA_REGISTER_NAMESPACE) == 0) {
				char buffer[960];
				snprintf(buffer, sizeof(buffer), "%s\t\t", prefix);
				Debug.cout("%15s{", prefix);
				luaT_dumpTable(L, lua_gettop(L), buffer);
				Debug.cout("%15s}", prefix);
			}

			lua_pop(L, 1);/* removes 'value'; keeps 'key' for next iteration */
		}
	}
	
	void luaT_dumpRootTable(lua_State* L) {
		lua_getglobal(L, "_G");
		Debug.cout("dump root table");
		Debug.cout("{");
		luaT_dumpTable(L, lua_gettop(L), "\t");
		Debug.cout("}");
		lua_pop(L, 1);/* remove `table` */
	}
	
	void luaT_dumpRegistryTable(lua_State* L) {
		lua_getregistry(L);
		Debug.cout("dump registry table");
		Debug.cout("{");
		luaT_dumpTable(L, lua_gettop(L), "\t");
		Debug.cout("}");
		lua_pop(L, 1);/* remove `table` */
	}
	
	void luaT_regNamespace(lua_State* L, const char* ns) {
		lua_getglobal(L, "_G");
		lua_pushstring(L, ns);
		lua_newtable(L);
		lua_rawset(L, -3);
		lua_pop(L, 1);
	}
	
	void luaT_beginNamespace(lua_State* L, const char* ns) {
		lua_getglobal(L, "_G");
		lua_pushstring(L, ns);
		lua_rawget(L, -2);
	}
	
	void luaT_endNamespace(lua_State* L) {
		lua_pop(L, 2);
	}
	
	void luaT_regFunction(lua_State* L, const char* func, lua_CFunction routine) {
		lua_pushstring(L, func);
		lua_pushcfunction(L, routine);
		lua_rawset(L, -3);
	}
	
	void luaT_regGlobalFunction(lua_State* L, const char* func, lua_CFunction routine) {
		lua_getglobal(L, "_G");
		luaT_regFunction(L, func, routine);
		lua_pop(L, 1);
	}
	
	void luaT_regInteger(lua_State* L, const char* key, int value) {
		lua_pushstring(L, key);
		lua_pushinteger(L, value);
		lua_settable(L, -3);
	}
	
	void luaT_regLong(lua_State* L, const char* key, long value) {
		lua_pushstring(L, key);
		lua_pushnumber(L, value);
		lua_settable(L, -3);
	}
	
	void luaT_regDouble(lua_State* L, const char* key, double value) {
		lua_pushstring(L, key);
		lua_pushnumber(L, value);
		lua_settable(L, -3);
	}
	
	void luaT_regString(lua_State* L, const char* key, const char* value) {
		lua_pushstring(L, key);
		lua_pushstring(L, value);
		lua_settable(L, -3);
	}

	void luaT_regObject(lua_State* L, const char* key, const void* value) {
		lua_pushstring(L, key);
		lua_pushlightuserdata(L, (void*) value);
		lua_rawset(L, -3);
	}

#if 0
template <typename T> T* luaT_get_object(lua_State* L, const char* name) {
	lua_getglobal(L, name);
	Assert(lua_islightuserdata(L, -1), "top: %d, type: %s", lua_gettop(L), lua_typename(L, lua_type(L, -1)));
	void* userdata = lua_touserdata(L, -1);
	lua_pop(L, 1);/* remove `userdata` */
	return static_cast<T*>(userdata);
}
#endif

	// boolean
	template <> void luaT_pushvalue(lua_State* L, bool value) {
		lua_pushboolean(L, value);
	}

	// integer
	template <> void luaT_pushvalue(lua_State* L, s8 value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, s16 value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, s32 value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, s64 value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, u8 value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, u16 value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, u32 value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, u64 value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, long long value) {
		lua_pushinteger(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, unsigned long long value) {
		lua_pushinteger(L, value);
	}

	// float
	template <> void luaT_pushvalue(lua_State* L, float value) {
		lua_pushnumber(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, double value) {
		lua_pushnumber(L, value);
	}

	// string
	template <> void luaT_pushvalue(lua_State* L, char* value) {
		lua_pushstring(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, const char* value) {
		lua_pushstring(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, std::string value) {
		lua_pushlstring(L, value.data(), value.length());
	}

	// lightuserdata
	template <> void luaT_pushvalue(lua_State* L, void* value) {
		lua_pushlightuserdata(L, value);
	}
	template <> void luaT_pushvalue(lua_State* L, const void* value) {
		lua_pushlightuserdata(L, (void*) value);
	}

	// luaT_Value
	template <> void luaT_pushvalue(lua_State* L, luaT_Value value) {
		switch (value.type) {
			default: Error << "unknown value.type: " << value.type;
			case LUA_TNIL: lua_pushnil(L); break;
			case LUA_TBOOLEAN: luaT_pushvalue(L, value.value_bool); break;
			case LUA_TINTEGER: luaT_pushvalue(L, value.value_integer); break;
			case LUA_TNUMBER: luaT_pushvalue(L, value.value_float); break;
			case LUA_TSTRING: luaT_pushvalue(L, value.value_string); break;
		}
	}
}


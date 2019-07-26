/*
 * \file: slam_lua.h
 * \brief: Created by hushouguo at 17:14:53 Jul 25 2019
 */
 
#ifndef __SLAM_LUA_H__
#define __SLAM_LUA_H__

#define	SLAM_LUA_USE_LUAJIT						1
#define SLAM_LUA_STACK_SIZE						8192
#define SLAM_LUA_REGISTER_NAMESPACE				"cc"

#define LUA_REGISTER(L, F, ROUTINE) \
	lua_pushstring(L, F);\
	lua_pushcfunction(L, ROUTINE);\
	lua_rawset(L, -3);

extern lua_State* slam_lua_newstate(size_t stack_size);
extern void slam_lua_closestate(lua_State* L);
	
extern void slam_lua_showversion(lua_State* L);
extern const char* slam_lua_tostring(lua_State* L, int idx);

extern bool slam_lua_pcall(lua_State* L, int args, slam_lua_value_t* lvalue);
extern bool slam_lua_dostring(lua_State* L, const char* str);
extern bool slam_lua_dofile(lua_State* L, const char* filename);

extern void slam_lua_cleanup(lua_State* L);
extern void slam_lua_tracestack(lua_State* L);

extern void slam_lua_dump_table(lua_State* L, int idx, const char* prefix);
extern void slam_lua_dump_root_table(lua_State* L);
extern void slam_lua_dump_registry_table(lua_State* L);	

extern void slam_lua_reg_namespace(lua_State* L, const char* ns);
extern void slam_lua_begin_namespace(lua_State* L, const char* ns);
extern void slam_lua_end_namespace(lua_State* L);

extern void slam_lua_reg_function(lua_State* L, const char* func, lua_CFunction routine);
extern void slam_lua_reg_global_function(lua_State* L, const char* func, lua_CFunction routine);	
extern void slam_lua_reg_integer(lua_State* L, const char* key, int value);
extern void slam_lua_reg_long(lua_State* L, const char* key, long value);
extern void slam_lua_reg_double(lua_State* L, const char* key, double value);
extern void slam_lua_reg_string(lua_State* L, const char* key, const char* value);
extern void slam_lua_reg_object(lua_State* L, const char* key, const void* value);

extern void slam_lua_pushvalue(lua_State* L, slam_lua_value_t* lvalue);

#endif

/*
 * \file: slam_lua.h
 * \brief: Created by hushouguo at 17:14:53 Jul 25 2019
 */
 
#ifndef __SLAM_LUA_H__
#define __SLAM_LUA_H__

#define	SLAM_LUA_USE_LUAJIT						1
#define SLAM_LUA_STACK_SIZE						8192
#define SLAM_LUA_REGISTER_NAMESPACE				"cc"
#define SLAM_LUA_METATABLE_DB_NAME              "cc.DB"


#define LUA_REGISTER(L, F, ROUTINE) \
	lua_pushstring(L, F);\
	lua_pushcfunction(L, ROUTINE);\
	lua_rawset(L, -3);


struct slam_lua_s {
    lua_State* L;
    const char* entryfile;
    size_t stack_size;
};    

typedef struct slam_lua_s slam_lua_t;

extern slam_lua_t* slam_lua_new(size_t stack_size);
extern void slam_lua_delete(slam_lua_t* lua);

extern bool slam_lua_load_entryfile(slam_lua_t* lua, const char* entryfile);
	
extern void slam_lua_showversion(slam_lua_t* lua);
extern const char* slam_lua_tostring(slam_lua_t* lua, int idx);

extern bool slam_lua_pcall(slam_lua_t* lua, int args, slam_lua_value_t* lvalue);
extern bool slam_lua_dostring(slam_lua_t* lua, const char* str);
extern bool slam_lua_dofile(slam_lua_t* lua, const char* filename);

extern void slam_lua_cleanup(slam_lua_t* lua);
extern void slam_lua_tracestack(slam_lua_t* lua);

extern void slam_lua_dump_table(lua_State* L, int idx, const char* prefix);
extern void slam_lua_dump_root_table(lua_State* L);

extern void slam_lua_reg_namespace(slam_lua_t* lua, const char* ns);
extern void slam_lua_begin_namespace(slam_lua_t* lua, const char* ns);
extern void slam_lua_end_namespace(slam_lua_t* lua);

extern void slam_lua_reg_function(slam_lua_t* lua, const char* func, lua_CFunction routine);
extern void slam_lua_reg_global_function(slam_lua_t* lua, const char* func, lua_CFunction routine);
extern void slam_lua_reg_integer(slam_lua_t* lua, const char* key, int value);
extern void slam_lua_reg_long(slam_lua_t* lua, const char* key, long value);
extern void slam_lua_reg_double(slam_lua_t* lua, const char* key, double value);
extern void slam_lua_reg_string(slam_lua_t* lua, const char* key, const char* value);
extern void slam_lua_reg_object(slam_lua_t* lua, const char* key, const void* value);

extern void slam_lua_pushvalue(slam_lua_t* lua, slam_lua_value_t* lvalue);

extern int slam_lua_ref(slam_lua_t* lua); // value at the top of stack, return key
extern void slam_lua_unref(slam_lua_t* lua, int ref); // unref from registry
extern void slam_lua_get_value_by_ref(slam_lua_t* lua, int ref); // return value to the top of stack
extern void slam_lua_dump_registry_table(slam_lua_t* lua);

#endif

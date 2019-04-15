/*
 * \file: luaT.h
 * \brief: Created by hushouguo at 09:32:30 Apr 15 2019
 */
 
#ifndef __LUAT_H__
#define __LUAT_H__

#define LUAT_ENABLE_DEBUG					1
#define	USE_LUAJIT							1
#define LUA_STACK_SIZE						8192
//#define LUA_REGISTER_NAMESPACE				"cc"
//#define LUA_REGISTER_SERVICE				"cc.service"

#define LUA_TINTEGER						9

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>

//luajit-2.0.5
#include "lua.hpp"

// google protobuf-3.6.1
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/message.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

namespace luaT {

	//
	// luaT_Value
	struct luaT_Value {
		int  type;
		bool value_bool;
		lua_Integer value_integer;
		lua_Number value_float;
		std::string value_string;
		// Note: not support table, userdata, function etc ...
		luaT_Value() : type(LUA_TNIL) {}
		luaT_Value(bool value) : type(LUA_TBOOLEAN), value_bool(value) {}
		luaT_Value(lua_Integer value) : type(LUA_TINTEGER), value_integer(value) {}
		luaT_Value(lua_Number value) : type(LUA_TNUMBER), value_float(value) {}
		luaT_Value(const char* value) : type(LUA_TSTRING), value_string(value) {}
		luaT_Value(const luaT_Value& value) { this->set(value); }
		inline void set(bool value) { this->type = LUA_TBOOLEAN; this->value_bool = value; }
		inline void set(lua_Integer value) { this->type = LUA_TINTEGER; this->value_integer = value; }
		inline void set(lua_Number value) { this->type = LUA_TNUMBER; this->value_float = static_cast<float>(value); }
		inline void set(const char* value) { this->type = LUA_TSTRING; this->value_string = value; }
		inline void set(const luaT_Value& value) { 
			this->type = value.type;
			this->value_bool = value.value_bool;
			this->value_integer = value.value_integer;
			this->value_float = value.value_float;
			this->value_string = value.value_string;
		}
		template <typename T> void operator = (T value) { this->set(value); }		
		inline bool isbool() { return this->type == LUA_TBOOLEAN; }
		inline bool isinteger() { return this->type == LUA_TINTEGER; }
		inline bool isfloat() { return this->type == LUA_TNUMBER; }
		inline bool isstring() { return this->type == LUA_TSTRING; }
		inline bool isnil() { return this->type == LUA_TNIL; }
	};	

	lua_State* luaT_newstate(uint32_t stackSize = LUA_STACK_SIZE);
	void luaT_close(lua_State*);

	void luaT_setOwner(lua_State*, const char* name, uint32_t sid);
	uint32_t luaT_getOwner(lua_State*, const char* name);
	
	void luaT_showversion(lua_State* L);
	const char* luaT_tostring(lua_State* L, int idx);
	
	bool luaT_pcall(lua_State* L, int args, luaT_Value& ret);
	void luaT_getRegistry(lua_State* L, int ref);
	void luaT_getGlobalFunction(lua_State* L, const char* func);
		
	bool luaT_execString(lua_State* L, const char* s);
	bool luaT_execFile(lua_State* L, const char* filename);
		
	void luaT_cleanup(lua_State* L);
	void luaT_tracestack(lua_State* L);
	
	void luaT_dumpTable(lua_State* L, int idx, const char* prefix);
	void luaT_dumpRootTable(lua_State* L);
	void luaT_dumpRegistryTable(lua_State* L);	
	
	void luaT_regNamespace(lua_State* L, const char* ns);
	void luaT_beginNamespace(lua_State* L, const char* ns);
	void luaT_endNamespace(lua_State* L);
	
	void luaT_regFunction(lua_State* L, const char* func, lua_CFunction routine);
	void luaT_regGlobalFunction(lua_State* L, const char* func, lua_CFunction routine);	
	void luaT_regInteger(lua_State* L, const char* key, int value);
	void luaT_regLong(lua_State* L, const char* key, long value);
	void luaT_regDouble(lua_State* L, const char* key, double value);
	void luaT_regString(lua_State* L, const char* key, const char* value);
	void luaT_regObject(lua_State* L, const char* key, const void* value);

	template <typename T> void luaT_pushvalue(lua_State* L, T value) {
		lua_pushnil(L);
		fprintf(stderr, "luaT_pushvalue template specialization failure for type: %s\n", typeid(T).name());
	}

	// boolean
	template <> void luaT_pushvalue(lua_State* L, bool value);

	// integer
	template <> void luaT_pushvalue(lua_State* L, int8_t value);
	template <> void luaT_pushvalue(lua_State* L, int16_t value);
	template <> void luaT_pushvalue(lua_State* L, int32_t value);
	template <> void luaT_pushvalue(lua_State* L, int64_t value);
	template <> void luaT_pushvalue(lua_State* L, uint8_t value);
	template <> void luaT_pushvalue(lua_State* L, uint16_t value);
	template <> void luaT_pushvalue(lua_State* L, uint32_t value);
	template <> void luaT_pushvalue(lua_State* L, uint64_t value);
	template <> void luaT_pushvalue(lua_State* L, long long value);
	template <> void luaT_pushvalue(lua_State* L, unsigned long long value);

	// float
	template <> void luaT_pushvalue(lua_State* L, float value);
	template <> void luaT_pushvalue(lua_State* L, double value);

	// string
	template <> void luaT_pushvalue(lua_State* L, char* value);
	template <> void luaT_pushvalue(lua_State* L, const char* value);
	template <> void luaT_pushvalue(lua_State* L, std::string value);

	// lightuserdata
	template <> void luaT_pushvalue(lua_State* L, void* value);
	template <> void luaT_pushvalue(lua_State* L, const void* value);

	// luaT_Value
	template <> void luaT_pushvalue(lua_State* L, luaT_Value value);
	
#define LUA_REGISTER(L, F, ROUTINE) \
	lua_pushstring(L, F);\
	lua_pushcfunction(L, ROUTINE);\
	lua_rawset(L, -3);

	
	bool luaT_callFunction(lua_State* L, luaT_Value&);
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	bool luaT_callFunction(lua_State* L, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, luaT_Value& ret) {
		luaT_pushvalue(L, t1);
		luaT_pushvalue(L, t2);
		luaT_pushvalue(L, t3);
		luaT_pushvalue(L, t4);
		luaT_pushvalue(L, t5);
		luaT_pushvalue(L, t6);
		return luaT_pcall(L, 6, ret);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	bool luaT_callFunction(lua_State* L, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, luaT_Value& ret) {
		luaT_pushvalue(L, t1);
		luaT_pushvalue(L, t2);
		luaT_pushvalue(L, t3);
		luaT_pushvalue(L, t4);
		luaT_pushvalue(L, t5);
		return luaT_pcall(L, 5, ret);
	}
	
	template <typename T1, typename T2, typename T3, typename T4>
	bool luaT_callFunction(lua_State* L, T1 t1, T2 t2, T3 t3, T4 t4, luaT_Value& ret) {
		luaT_pushvalue(L, t1);
		luaT_pushvalue(L, t2);
		luaT_pushvalue(L, t3);
		luaT_pushvalue(L, t4);
		return luaT_pcall(L, 4, ret);
	}
	
	template <typename T1, typename T2, typename T3>
	bool luaT_callFunction(lua_State* L, T1 t1, T2 t2, T3 t3, luaT_Value& ret) {
		luaT_pushvalue(L, t1);
		luaT_pushvalue(L, t2);
		luaT_pushvalue(L, t3);
		return luaT_pcall(L, 3, ret);
	}
	
	template <typename T1, typename T2>
	bool luaT_callFunction(lua_State* L, T1 t1, T2 t2, luaT_Value& ret) {
		luaT_pushvalue(L, t1);
		luaT_pushvalue(L, t2);
		return luaT_pcall(L, 2, ret);
	}
	
	template <typename T1>
	bool luaT_callFunction(lua_State* L, T1 t1, luaT_Value& ret) {
		luaT_pushvalue(L, t1);
		return luaT_pcall(L, 1, ret);
	}

	class ByteBuffer;
	
	//
	// json
	bool luaT_json_parser_decode(lua_State* L, const char* jsonstr, unsigned int len);
	bool luaT_json_parser_encode(lua_State* L, ByteBuffer* byteBuffer);

	//
	// xml
	bool luaT_xml_parser_decode(lua_State* L, const char* file);
	const char* luaT_xml_parser_encode(lua_State* L);

	//
	// google protobuf message
	class luaT_message_parser {
		public:
			virtual ~luaT_message_parser() = 0;

		public:
			virtual bool loadmsg(const char* filename) = 0; // filename also is directory
			virtual bool regmsg(uint32_t msgid, const char* name) = 0;
			virtual bool encode(lua_State* L, uint32_t msgid, void* buf, size_t& bufsize) = 0;
			virtual bool encode(lua_State* L, uint32_t msgid, std::string& out) = 0;
			virtual bool decode(lua_State* L, uint32_t msgid, const void* buf, size_t bufsize) = 0;
			virtual bool decode(lua_State* L, uint32_t msgid, const std::string& in) = 0;
			virtual google::protobuf::Message* encode(lua_State* L, uint32_t msgid) = 0;
	};

	struct luaT_message_parser_creator {
		static luaT_message_parser* create();
	};	
}

#endif

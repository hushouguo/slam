/*
 * \file: slam_lua_value.c
 * \brief: Created by hushouguo at 11:17:18 Jul 26 2019
 */

#include "slam.h"

/* #define LUA_TNIL        		0 */
/* #define LUA_TBOOLEAN        	1 */
/* #define LUA_TLIGHTUSERDATA  	2 */
/* #define LUA_TNUMBER     		3 */
/* #define LUA_TSTRING     		4 */
/* #define LUA_TTABLE      		5 */
/* #define LUA_TFUNCTION       	6 */
/* #define LUA_TUSERDATA       	7 */
/* #define LUA_TTHREAD     		8 */
#define LUA_TINTEGER			9

struct slam_lua_value_s {
	int  type; /* default: nil */
	union {
		bool boolean;
		lua_Integer integer; /* uint64_t */
		lua_Number number; /* double */
		char* string; /* strdup */
	/* NOTE: not support lightuserdata, table, function, userdata, thread */
	};
};	

slam_lua_value_t* slam_lua_value_new() {
	slam_lua_value_t* lvalue = (slam_lua_value_t *) slam_malloc(sizeof(slam_lua_value_t));
	lvalue->string = nullptr;
	lvalue->type = LUA_TNIL;
	return lvalue;
}

void slam_lua_value_delete(slam_lua_value_t* lvalue) {
	if (lvalue->type == LUA_TSTRING) {
		assert(lvalue->string != nullptr);
		slam_free(lvalue->string);
	}
	slam_free(lvalue);
}

void slam_lua_value_set_nil(slam_lua_value_t* lvalue) {
	lvalue->type = LUA_TNIL;
}
void slam_lua_value_set_boolean(slam_lua_value_t* lvalue, bool value) {
	lvalue->type = LUA_TBOOLEAN;
	lvalue->boolean = value;
}
void slam_lua_value_set_integer(slam_lua_value_t* lvalue, lua_Integer value) {
	lvalue->type = LUA_TINTEGER;
	lvalue->integer = value;
}
void slam_lua_value_set_number(slam_lua_value_t* lvalue, lua_Number value) {
	lvalue->type = LUA_TNUMBER;
	lvalue->number = value;
}
void slam_lua_value_set_string(slam_lua_value_t* lvalue, const char* value) {
	lvalue->type = LUA_TSTRING;
	lvalue->string = strdup(value);
}

bool slam_lua_value_is_nil(slam_lua_value_t* lvalue) {
	return lvalue->type == LUA_TNIL;
}
bool slam_lua_value_is_boolean(slam_lua_value_t* lvalue) {
	return lvalue->type == LUA_TBOOLEAN;
}
bool slam_lua_value_is_integer(slam_lua_value_t* lvalue) {
	return lvalue->type == LUA_TINTEGER;
}
bool slam_lua_value_is_number(slam_lua_value_t* lvalue) {
	return lvalue->type == LUA_TNUMBER;
}
bool slam_lua_value_is_string(slam_lua_value_t* lvalue) {
	return lvalue->type == LUA_TSTRING;
}

bool slam_lua_value_get_boolean(slam_lua_value_t* lvalue) {
	assert(lvalue->type == LUA_TBOOLEAN);
	return lvalue->boolean;
}
lua_Integer slam_lua_value_get_integer(slam_lua_value_t* lvalue) {
	assert(lvalue->type == LUA_TINTEGER);
	return lvalue->integer;
}
lua_Number slam_lua_value_get_number(slam_lua_value_t* lvalue) {
	assert(lvalue->type == LUA_TNUMBER);
	return lvalue->number;
}
const char* slam_lua_value_get_string(slam_lua_value_t* lvalue) {
	assert(lvalue->type == LUA_TSTRING);
	return lvalue->string;
}

int slam_lua_value_type(slam_lua_value_t* lvalue) {
	return lvalue->type;
}


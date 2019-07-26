/*
 * \file: slam_lua_value.h
 * \brief: Created by hushouguo at 11:17:09 Jul 26 2019
 */
 
#ifndef __SLAM_LUA_VALUE_H__
#define __SLAM_LUA_VALUE_H__

typedef struct slam_lua_value_s slam_lua_value_t;

extern slam_lua_value_t* slam_lua_value_new();
extern void slam_lua_value_delete(slam_lua_value_t* lvalue);

extern void slam_lua_value_set_nil(slam_lua_value_t* lvalue);
extern void slam_lua_value_set_boolean(slam_lua_value_t* lvalue, bool value);
extern void slam_lua_value_set_integer(slam_lua_value_t* lvalue, lua_Integer value);
extern void slam_lua_value_set_number(slam_lua_value_t* lvalue, lua_Number value);
extern void slam_lua_value_set_string(slam_lua_value_t* lvalue, const char* value);

extern bool slam_lua_value_is_nil(slam_lua_value_t* lvalue);
extern bool slam_lua_value_is_boolean(slam_lua_value_t* lvalue);
extern bool slam_lua_value_is_integer(slam_lua_value_t* lvalue);
extern bool slam_lua_value_is_number(slam_lua_value_t* lvalue);
extern bool slam_lua_value_is_string(slam_lua_value_t* lvalue);

extern bool slam_lua_value_get_boolean(slam_lua_value_t* lvalue);
extern lua_Integer slam_lua_value_get_integer(slam_lua_value_t* lvalue);
extern lua_Number slam_lua_value_get_number(slam_lua_value_t* lvalue);
extern const char* slam_lua_value_get_string(slam_lua_value_t* lvalue);

extern int slam_lua_value_type(slam_lua_value_t* lvalue);

#endif

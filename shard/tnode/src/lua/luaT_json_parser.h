/*
 * \file: luaT_json_parser.h
 * \brief: Created by hushouguo at Nov 06 2014 18:20:00
 */
 
#ifndef __LUAT_JSON_PARSER_H__
#define __LUAT_JSON_PARSER_H__

BEGIN_NAMESPACE_TNODE {
	bool luaT_json_parser_decode(lua_State* L, const char* jsonstr, unsigned int len);
	bool luaT_json_parser_encode(lua_State* L, ByteBuffer* byteBuffer);
}

#endif

/*
 * \file: luaT_xml_parser.h
 * \brief: Created by hushouguo at Nov 06 2014 17:51:52
 */
 
#ifndef __LUAT_XML_PARSER_H__
#define __LUAT_XML_PARSER_H__

BEGIN_NAMESPACE_TNODE {
	bool luaT_xml_parser_decode(lua_State* L, const char* file);
	const char* luaT_xml_parser_encode(lua_State* L);
}

#endif

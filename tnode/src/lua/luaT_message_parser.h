/*
 * \file: luaT_message_parser.h
 * \brief: Created by hushouguo at 17:01:33 Aug 09 2017
 */
 
#ifndef __LUAT_MESSAGE_PARSER_H__
#define __LUAT_MESSAGE_PARSER_H__

BEGIN_NAMESPACE_TNODE {
	//
	// encode lua's table to buffer
	bool luaT_message_parser_encode(MessageParser* parser, lua_State* L, u32 msgid, std::string& out);
	bool luaT_message_parser_encode(MessageParser* parser, lua_State* L, u32 msgid, void* buf, size_t& bufsize);
	Message* luaT_message_parser_encode(MessageParser* parser, lua_State* L, u32 msgid);
	
	//
	// decode buffer to lua's table
	bool luaT_message_parser_decode(MessageParser* parser, lua_State* L, u32 msgid, const std::string& in);
	bool luaT_message_parser_decode(MessageParser* parser, lua_State* L, u32 msgid, const void* buf, size_t bufsize);
	bool luaT_message_parser_decode(MessageParser* parser, lua_State* L, Message* message);
}

#endif

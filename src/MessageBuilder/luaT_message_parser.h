/*
 * \file: luaT_message_parser.h
 * \brief: Created by hushouguo at 16:59:54 Jul 30 2019
 */
 
#ifndef __LUAT_MESSAGE_PARSER_H__
#define __LUAT_MESSAGE_PARSER_H__

//
// decode buffer to lua's table
bool luaT_message_parser_decode(MessageParser* parser, lua_State* L, msgid_t msgid, const void* buf, size_t bufsize);

//
// encode lua's table to buffer
void* luaT_message_parser_encode(MessageParser* parser, lua_State* L, msgid_t msgid, size_t& size);

#endif

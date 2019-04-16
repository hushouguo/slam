/*
 * \file: luaT_message_parser.h
 * \brief: Created by hushouguo at 17:01:33 Aug 09 2017
 */
 
#ifndef __LUAT_MESSAGE_PARSER_H__
#define __LUAT_MESSAGE_PARSER_H__

BEGIN_NAMESPACE_TNODE {
	class luaT_message_parser {
		public:
			virtual ~luaT_message_parser() = 0;

		public:
			//
			// load `proto` file or folder containing `proto` files
			virtual bool loadmsg(const char* filename) = 0; // filename also is directory
			//
			// register protocol name for msgid, such as: regmsg(1, `protocol.RequestMessage`)
			virtual bool regmsg(u32 msgid, const char* name) = 0;
			//
			// encode table of lua to buffer
			virtual bool encode(lua_State* L, u32 msgid, void* buf, size_t& bufsize) = 0;
			virtual bool encode(lua_State* L, u32 msgid, std::string& out) = 0;
			//
			// decode buffer to lua
			virtual bool decode(lua_State* L, u32 msgid, const void* buf, size_t bufsize) = 0;
			virtual bool decode(lua_State* L, u32 msgid, const std::string& in) = 0;
			
			//
			// encode table of lua to protobuf::Message
			virtual google::protobuf::Message* encode(lua_State* L, u32 msgid) = 0;
			//
			// encode table of lua, and allocate new protobuf::Message to return 
			virtual google::protobuf::Message* encode_newmsg(lua_State* L, u32 msgid) = 0;
			//
			// decode protobuf::Message to lua
			virtual bool decode(lua_State* L, google::protobuf::Message*) = 0;
			//
			// decode buffer to new protobuf::Message
			virtual google::protobuf::Message* decode(u32 msgid, const void* buf, size_t bufsize) = 0;
	};

	struct luaT_message_parser_creator {
		static luaT_message_parser* create();
	};
}

#endif

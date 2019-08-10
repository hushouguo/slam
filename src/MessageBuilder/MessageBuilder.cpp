/*
 * \file: MessageBuilder.cpp
 * \brief: Created by hushouguo at 14:30:57 Aug 06 2019
 */

#include "protocol.h"

static MessageParser* __parser = nullptr;

void MessageBuilderInit() {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;    
    SafeDelete(__parser);
    __parser = new MessageParser();
}

void MessageBuilderDestroy() {
    SafeDelete(__parser);    
    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
}

int MessageBuilderLoadDescriptor(const char* filename) { 
    CHECK_RETURN(__parser, BUILDER_ERROR, "MessageBuilder not init");
    CHECK_RETURN(filename, BUILDER_ERROR, "filename is nullptr");
	return __parser->LoadMessageDescriptor(filename) ? BUILDER_OK : BUILDER_ERROR;
}

int MessageBuilderRegister(msgid_t msgid, const char* name) {
    CHECK_RETURN(__parser, BUILDER_ERROR, "MessageBuilder not init");
	return __parser->RegisteMessage(msgid, name) ? BUILDER_OK : BUILDER_ERROR;
}

/* decode buffer to lua's table */
int MessageBuilderDecode(lua_State* L, msgid_t msgid, const void* buf, size_t bufsize) {
    CHECK_RETURN(__parser, BUILDER_ERROR, "MessageBuilder not init");
    CHECK_RETURN(buf, BUILDER_ERROR, "buf is nullptr");
    CHECK_RETURN(bufsize > 0, BUILDER_ERROR, "bufsize is 0");
    return luaT_message_parser_decode(__parser, L, msgid, buf, bufsize) ? BUILDER_OK : BUILDER_ERROR;
}

/* encode lua's table to buffer */
void* MessageBuilderEncode(lua_State* L, msgid_t msgid, size_t* size) {
    CHECK_RETURN(__parser, nullptr, "MessageBuilder not init");
    CHECK_RETURN(size, nullptr, "size is nullptr");
    return luaT_message_parser_encode(__parser, L, msgid, *size);
}


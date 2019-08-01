/*
 * \file: protocol.cpp
 * \brief: Created by hushouguo at 15:21:03 Jul 29 2019
 */

#include "protocol.h"

static MessageParser* __parser = nullptr;

__attribute__((constructor)) static void __protocol_init() {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}

__attribute__((destructor)) static void __protocol_destroy() {
	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();
}

int protocol_load_descriptor(const char* filename) {
	if (!__parser) {
		__parser = new MessageParser();
	}
	if (!filename || !__parser->LoadMessageDescriptor(filename)) {
		return -1;
	}
	return 0;
}

int protocol_reg_message(msgid_t msgid, const char* name) {
	if (!__parser) {
		return -1;
	}
	return __parser->RegisteMessage(msgid, name) ? 0 : -1;
}

/* decode socket->rbuffer to lua's table */
ssize_t protocol_decode(lua_State* L, msgid_t* msgid, const void* buf, size_t bufsize) {
	if (!__parser) {
		return -1;
	}
	if (bufsize < sizeof(rawmessage_t)) {
		return 0;	// not enough data
	}
	
	rawmessage_t* rawmsg = (rawmessage_t *) buf;
	
	if (rawmsg->len < sizeof(rawmessage_t)) {
		return -1;	// illegal package
	}	
	if (bufsize < rawmsg->len) {
		return 0;	// package is incomplete
	}

	// Debug("decode msgid: %d, rawmsg->len: %d, sizeof(rawmessage_t): %ld, bufsize: %ld", rawmsg->id, rawmsg->len, sizeof(rawmessage_t), bufsize);

	if (!luaT_message_parser_decode(__parser, L, rawmsg->id, rawmsg->payload, bufsize - sizeof(rawmessage_t))) {
		return -1;	// parse package error
	}
	
	if (msgid) {
		*msgid = rawmsg->id;
	}
	
	return rawmsg->len;
}

/* encode lua's table to socket->wbuffer */
ssize_t protocol_encode(lua_State* L, msgid_t msgid, void* buf, size_t bufsize) {	
	if (!__parser) {
		return -1;
	}

	rawmessage_t* rawmsg = (rawmessage_t *) buf;
	bufsize -= sizeof(rawmessage_t);

	size_t payload_size = bufsize;
	if (!luaT_message_parser_encode(__parser, L, msgid, rawmsg->payload, payload_size)) {
		assert(payload_size >= bufsize);
		return payload_size == bufsize ? -1 : (payload_size + sizeof(rawmessage_t)); // parse package error or bufsize too small
	}

	rawmsg->len = sizeof(rawmessage_t) + payload_size;
	rawmsg->entityid = 0;
	rawmsg->id = msgid;
	rawmsg->flags = 0;

	// Debug("encode msgid: %d, sizeof(rawmessage_t): %ld, payload_size: %ld, rawmsg->len: %d", msgid, sizeof(rawmessage_t), payload_size, rawmsg->len);

	return rawmsg->len;
}


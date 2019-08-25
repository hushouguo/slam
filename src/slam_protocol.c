/*
 * \file: slam_protocol.c
 * \brief: Created by hushouguo at 10:42:02 Jul 26 2019
 */

#include "slam.h"

#define MESSAGE_REF_NAME(msgid, typename)	\
    ({ char __ref_name[PATH_MAX]; snprintf(__ref_name, sizeof(__ref_name), "%s.%d", typename, msgid); __ref_name; })

slam_protocol_t* slam_protocol_new() {
    MessageBuilderInit();
    slam_protocol_t* protocol = (slam_protocol_t *) slam_malloc(sizeof(slam_protocol_t));
    memset(protocol->regtable, 0, sizeof(protocol->regtable));
    return protocol;
}

void slam_protocol_delete(slam_protocol_t* protocol) {
	uint32_t i = 0;
	for (i = 0; i < MAX_MESSAGE_ID; ++i) {
		slam_free(protocol->regtable[i]);
	}
	slam_free(protocol);
    MessageBuilderDestroy();
}

/* load proto descriptor */
bool slam_protocol_load_descriptor(slam_protocol_t* protocol, const char* filename) {
    return MessageBuilderLoadDescriptor(filename) == BUILDER_OK;
}

/* registe message */
const char* slam_protocol_reg_message(slam_protocol_t* protocol, msgid_t msgid, const char* typename) {
	CHECK_RETURN(msgid >= 0, nullptr, "illegal msgid: %d", msgid);
	CHECK_RETURN(msgid < MAX_MESSAGE_ID, nullptr, "msgid: %d overflow, max: %d", msgid, MAX_MESSAGE_ID);
	CHECK_RETURN(protocol->regtable[msgid] == nullptr, nullptr, "msgid: %d duplicate reg", msgid);
    if (MessageBuilderRegister(msgid, typename) != BUILDER_OK) {
        return nullptr;
    }
    return protocol->regtable[msgid] = strdup(MESSAGE_REF_NAME(msgid, typename));
}

/* msgid => registry value */
const char* slam_protocol_find_name(slam_protocol_t* protocol, msgid_t msgid) {
	CHECK_RETURN(msgid >= 0, nullptr, "illegal msgid: %d", msgid);
	CHECK_RETURN(msgid < MAX_MESSAGE_ID, nullptr, "msgid: %d overflow, max: %d", msgid, MAX_MESSAGE_ID);
	CHECK_RETURN(protocol->regtable[msgid] != nullptr, nullptr, "msgid: %d not reg", msgid);
	return protocol->regtable[msgid];
}

/* decode buffer to lua's table */
bool slam_protocol_decode(slam_runnable_t* runnable, const slam_message_t* message) {
    assert(message);
	assert(message->bytesize == message->bufsize);
	slam_rawmessage_t* rawmessage = (slam_rawmessage_t *) message->buf;
	assert(rawmessage->len >= sizeof(slam_rawmessage_t));
	int rc = MessageBuilderDecode(
    	                runnable->lua->L, 
    	                rawmessage->msgid, 
    	                rawmessage->payload, 
    	                rawmessage->len - sizeof(slam_rawmessage_t)
	                );
    CHECK_RETURN(rc == BUILDER_OK, false, "DecodeMessage: %d error, len: %ld", rawmessage->msgid, rawmessage->len - sizeof(slam_rawmessage_t));
	return slam_lua_event_message(runnable, message->fd, rawmessage->msgid); /* notify event.message to lua */
}

/* encode lua's table to buffer */
slam_message_t* slam_protocol_encode(slam_runnable_t* runnable, SOCKET fd, msgid_t msgid) {
    size_t head_size = sizeof(slam_message_t) + sizeof(slam_rawmessage_t);
    size_t size = head_size; // reserved size
	log_trace("func: %s, fd: %d, msgid: %d", __FUNCTION__, fd, msgid);
    void* buf = MessageBuilderEncode(
                        runnable->lua->L,
                        msgid,
                        &size
                    );
    CHECK_RETURN(buf, nullptr, "EncodeMessage: %d error, size: %ld", msgid, size);
    assert(size >= head_size);
    log_trace("EncodeMessage: %d, from %ld to %ld", msgid, head_size, size);
    slam_message_t* message = (slam_message_t *) buf;
    message->fd = fd;
    message->type = MESSAGE_TYPE_PROTOBUF_PACKAGE;
    message->bufsize = size - sizeof(slam_message_t);
    message->bytesize = 0;
    slam_rawmessage_t* rawmessage = (slam_rawmessage_t *) message->buf;
    rawmessage->len = message->bufsize;
    rawmessage->entityid = 0;
    rawmessage->msgid = msgid;    
    rawmessage->flags = 0;
    CHECK_ALARM(rawmessage->len < __slam_main->max_package_size, "rawmessage->len: %d, max_package_size: %ld", rawmessage->len, __slam_main->max_package_size);
    return message;
}


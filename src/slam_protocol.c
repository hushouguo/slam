/*
 * \file: slam_protocol.c
 * \brief: Created by hushouguo at 10:42:02 Jul 26 2019
 */

#include "slam.h"

#define SYMBOL_PROTOCOL_LOAD_DESCRIPTOR		"protocol_load_descriptor"
#define SYMBOL_PROTOCOL_REG_MESSAGE			"protocol_reg_message"
#define SYMBOL_PROTOCOL_DECODE				"protocol_decode"
#define SYMBOL_PROTOCOL_ENCODE				"protocol_encode"

typedef int (*PROTOCOL_LOAD_DESCRIPTOR)(const char* filename);
typedef int (*PROTOCOL_REG_MESSAGE)(int msgid, const char* name);
typedef ssize_t (*PROTOCOL_DECODE)(lua_State* L, int* msgid, const void* buf, size_t bufsize);
typedef ssize_t (*PROTOCOL_ENCODE)(lua_State* L, int msgid, void* buf, size_t bufsize);

#define INIT_PROTOCOL_ENCODE_MESSAGE_SIZE	4*KB

struct slam_protocol_s {
	const char* dynamic_lib_name;
	void* dynamic_lib_handle;
	PROTOCOL_LOAD_DESCRIPTOR dl_protocol_load_descriptor;
	PROTOCOL_REG_MESSAGE dl_protocol_reg_message;
	PROTOCOL_DECODE dl_protocol_decode;
	PROTOCOL_ENCODE dl_protocol_encode;
};

slam_protocol_t* slam_protocol_new() {
	slam_protocol_t* protocol = (slam_protocol_t *) malloc(sizeof(slam_protocol_t));
	protocol->dynamic_lib_name = nullptr;
	protocol->dynamic_lib_handle = nullptr;
	protocol->dl_protocol_load_descriptor = nullptr;
	protocol->dl_protocol_reg_message = nullptr;
	protocol->dl_protocol_decode = nullptr;
	protocol->dl_protocol_encode = nullptr;
	return protocol;
}

void slam_protocol_delete(slam_protocol_t* protocol) {
	slam_free(protocol->dynamic_lib_name);
	if (protocol->dynamic_lib_handle != nullptr) {
		dlclose(protocol->dynamic_lib_handle);
	}
	slam_free(protocol);
}

bool slam_protocol_load_dynamic_lib(slam_protocol_t* protocol, const char* dynamic_lib) {
	if (protocol->dynamic_lib_handle != nullptr) {
		dlclose(protocol->dynamic_lib_handle);
	}
	protocol->dynamic_lib_handle = dlopen(dynamic_lib, RTLD_LAZY);
	CHECK_RETURN(protocol->dynamic_lib_handle, false, "dlopen: %s, error: %s", dynamic_lib, dlerror());
	protocol->dl_protocol_load_descriptor = dlsym(protocol->dynamic_lib_handle, SYMBOL_PROTOCOL_LOAD_DESCRIPTOR);
	CHECK_RETURN(protocol->dl_protocol_load_descriptor, false, "dlsym: %s, error: %s", SYMBOL_PROTOCOL_LOAD_DESCRIPTOR, dlerror());
	protocol->dl_protocol_reg_message = dlsym(protocol->dynamic_lib_handle, SYMBOL_PROTOCOL_REG_MESSAGE);
	CHECK_RETURN(protocol->dl_protocol_reg_message, false, "dlsym: %s, error: %s", SYMBOL_PROTOCOL_REG_MESSAGE, dlerror());
	protocol->dl_protocol_decode = dlsym(protocol->dynamic_lib_handle, SYMBOL_PROTOCOL_DECODE);
	CHECK_RETURN(protocol->dl_protocol_decode, false, "dlsym: %s, error: %s", SYMBOL_PROTOCOL_DECODE, dlerror());
	protocol->dl_protocol_encode = dlsym(protocol->dynamic_lib_handle, SYMBOL_PROTOCOL_ENCODE);
	CHECK_RETURN(protocol->dl_protocol_encode, false, "dlsym: %s, error: %s", SYMBOL_PROTOCOL_ENCODE, dlerror());
	slam_free(protocol->dynamic_lib_name);
	protocol->dynamic_lib_name = strdup(dynamic_lib);
	return true;
}

bool slam_protocol_reload_dynamic_lib(slam_protocol_t* protocol, const char* dynamic_lib) {
	return slam_protocol_load_dynamic_lib(protocol, dynamic_lib);
}

/* load proto descriptor */
bool slam_protocol_load_descriptor(slam_runnable_t* runnable, const char* filename) {
	slam_protocol_t* protocol = slam_runnable_protocol(runnable);
	CHECK_RETURN(protocol, false, "runnable not protocol instance for load descriptor");
	CHECK_RETURN(protocol->dl_protocol_load_descriptor, false, "not export symbol: %s", SYMBOL_PROTOCOL_LOAD_DESCRIPTOR);
	return protocol->dl_protocol_load_descriptor(filename) == 0;
}

/* registe message */
bool slam_protocol_reg_message(slam_runnable_t* runnable, msgid_t msgid, const char* name) {
	slam_protocol_t* protocol = slam_runnable_protocol(runnable);
	CHECK_RETURN(protocol, false, "runnable not protocol instance for registe message");
	CHECK_RETURN(protocol->dl_protocol_reg_message, false, "not export symbol: %s", SYMBOL_PROTOCOL_REG_MESSAGE);
	return protocol->dl_protocol_reg_message(msgid, name) == 0;
}

/* decode socket->rbuffer to lua's table */
bool slam_protocol_decode(slam_runnable_t* runnable, const slam_socket_t* socket) {
	slam_protocol_t* protocol = slam_runnable_protocol(runnable);
	CHECK_RETURN(protocol, false, "runnable not protocol instance for decode");
	CHECK_RETURN(protocol->dl_protocol_decode, false, "not export symbol: %s", SYMBOL_PROTOCOL_DECODE);
	while (true) {
		msgid_t msgid = 0;
		ssize_t ssize = protocol->dl_protocol_decode(
							slam_luastate(slam_runnable_lua(runnable)),
							&msgid,
							slam_byte_buffer_readbuffer(socket->rbuffer),
							slam_byte_buffer_size(socket->rbuffer)
							);
		if (ssize > 0) {
			slam_lua_event_message(runnable, socket->fd, msgid); /* notify event.message to lua */
			slam_byte_buffer_readlength(socket->rbuffer, ssize);
		}
		else if (ssize < 0) {
			Error("symbol: %s return : %ld", SYMBOL_PROTOCOL_DECODE, ssize);
			return false;
		}
		else {
			break; /* no more messages */
		}
	}
	return true;
}

/* encode lua's table to socket->wbuffer */
bool slam_protocol_encode(slam_runnable_t* runnable, msgid_t msgid, slam_socket_t* socket) {
	slam_protocol_t* protocol = slam_runnable_protocol(runnable);
	CHECK_RETURN(protocol, false, "runnable not protocol instance for encode");
	CHECK_RETURN(protocol->dl_protocol_encode, false, "not export symbol: %s", SYMBOL_PROTOCOL_ENCODE);
	while (true) {
		ssize_t message_size = INIT_PROTOCOL_ENCODE_MESSAGE_SIZE;
		ssize_t ssize = protocol->dl_protocol_encode(
							slam_luastate(slam_runnable_lua(runnable)), 
							msgid,
							slam_byte_buffer_writebuffer(socket->wbuffer, message_size),
							message_size
							);
		CHECK_RETURN(ssize >= 0, false, "symbol: %s return : %ld", SYMBOL_PROTOCOL_DECODE, ssize);
		if (ssize > message_size) {
			message_size = ssize; /* the message's size beyond message_size, reset and try again */
		}
		else {
			slam_byte_buffer_writelength(socket->wbuffer, ssize);
			break;
		}
	}
	return true;
}


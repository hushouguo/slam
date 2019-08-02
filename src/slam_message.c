/*
 * \file: slam_message.c
 * \brief: Created by hushouguo at 10:50:47 Aug 02 2019
 */

#include "slam.h"

#define MAX_MESSAGE_ID	65536

#define MESSAGE_REF_NAME(msgid, typename)	\
    ({ char __ref_name[PATH_MAX]; snprintf(__ref_name, sizeof(__ref_name), "%s.%d", typename, msgid); __ref_name; })

struct slam_message_s {
	const char* regtable[MAX_MESSAGE_ID];
};

slam_message_t* slam_message_new() {
	slam_message_t* message = (slam_message_t *) slam_malloc(sizeof(slam_message_t));
	memset(message, 0, sizeof(slam_message_t));
	return message;
}

void slam_message_delete(slam_message_t* message) {
	uint32_t i = 0;
	for (i = 0; i < MAX_MESSAGE_ID; ++i) {
		slam_free(message->regtable[i]);
	}
	slam_free(message);
}

const char* slam_message_reg(slam_message_t* message, msgid_t msgid, const char* typename) {
	CHECK_RETURN(msgid >= 0, nullptr, "illegal msgid: %d", msgid);
	CHECK_RETURN(msgid < MAX_MESSAGE_ID, nullptr, "msgid: %d overflow, max: %d", msgid, MAX_MESSAGE_ID);
	CHECK_RETURN(message->regtable[msgid] == nullptr, nullptr, "msgid: %d duplicate reg", msgid);
	return message->regtable[msgid] = strdup(MESSAGE_REF_NAME(msgid, typename));
}

const char* slam_message_find(slam_message_t* message, msgid_t msgid) {
	CHECK_RETURN(msgid >= 0, nullptr, "illegal msgid: %d", msgid);
	CHECK_RETURN(msgid < MAX_MESSAGE_ID, nullptr, "msgid: %d overflow, max: %d", msgid, MAX_MESSAGE_ID);
	CHECK_RETURN(message->regtable[msgid] != nullptr, nullptr, "msgid: %d not reg", msgid);
	return message->regtable[msgid];
}



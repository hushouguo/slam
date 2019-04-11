/*
 * \file: RecordMessage.h
 * \brief: Created by hushouguo at 16:02:44 Apr 10 2019
 */
 
#ifndef __RECORDMESSAGE_H__
#define __RECORDMESSAGE_H__

BEGIN_NAMESPACE_TNODE {

#define MAX_MESSAGE_ID		((u16)(-1))
#define MAX_MESSAGE_SIZE	((u32)(-1))

#pragma pack(push, 1)
	struct RecordMessage {
		u32 len;
		u64 entityid;
		u16 msgid;
		u16 flags;
		Byte payload[0];
	};
#pragma pack(pop)

	
#if false
#define NEW_MSG(STRUCTURE, ...)	\
		char __message_buffer__[SIZE];\
		STRUCTURE* msg = Constructor((STRUCTURE *)(__message_buffer__), ##__VA_ARGS__);
#endif	
	
#define DECLARE_MESSAGE() \
		typedef bool (*MESSAGE_ROUTINE)(NetworkInterface* task, int fd, rawmessage* rawmsg);\
		struct MessageTable {\
			MESSAGE_ROUTINE table[MAX_MESSAGE_ID];\
		};\
		static MessageTable __t;\
		__attribute__((constructor)) static void __t_init() {\
			memset(__t.table, 0, sizeof(__t.table));\
		}
		
	
#define ON_MSG(MSGID, STRUCTURE) \
		static bool onMessage_raw_##STRUCTURE(NetworkInterface* task, int fd, rawmessage* rawmsg);\
		static void onMessage_##STRUCTURE(NetworkInterface* task, int fd, STRUCTURE* msg, rawmessage* rawmsg);\
		__attribute__((constructor)) static void __##STRUCTURE() \
		{\
			assert(MSGID >= 0 && MSGID < MAX_MESSAGE_ID);\
			if (__t.table[MSGID]) {\
				printf("Duplicate message id:%d, %s\n", MSGID, #STRUCTURE);\
				abort();\
			}\
			else {\
				__t.table[MSGID] = onMessage_raw_##STRUCTURE;\
			}\
		}\
		static bool onMessage_raw_##STRUCTURE(NetworkInterface* task, int fd, rawmessage* rawmsg) {\
			STRUCTURE msg;\
			bool rc = msg.ParseFromArray(rawmsg->data, rawmsg->len - sizeof(rawmessage));\
			CHECK_RETURN(rc, false, "%s ParseFromArray failure:%d", #STRUCTURE, rawmsg->len);\
			onMessage_##STRUCTURE(task, fd, &msg, rawmsg);\
			return rc;\
		}\
		static void onMessage_##STRUCTURE(NetworkInterface* task, int fd, STRUCTURE* msg, rawmessage* rawmsg)
		
	
	
#define DISPATCH_MESSAGE(task, fd, rawmsg) \
		({\
			bool rc = false;\
			if (rawmsg->id < 0 || rawmsg->id >= MAX_MESSAGE_ID) {\
				log_error("illegal rawmsg->id:%d", rawmsg->id);\
			}\
			else {\
				rc = true;\
				if (__t.table[rawmsg->id]) {\
					rc = __t.table[rawmsg->id](task, fd, rawmsg);\
				}\
				else {\
					log_error("unhandled rawmsg:%d", rawmsg->id);\
				}\
			}\
			rc;\
		})	
}

#endif

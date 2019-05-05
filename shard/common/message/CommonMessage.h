/*
 * \file: CommonMessage.h
 * \brief: Created by hushouguo at 16:02:44 Apr 10 2019
 */
 
#ifndef __COMMON_MESSAGE_H__
#define __COMMON_MESSAGE_H__

BEGIN_NAMESPACE_SLAM {

#define MAX_MESSAGE_ID		((u16)(-1))
#define MAX_MESSAGE_SIZE	((u32)(-1))

#pragma pack(push, 1)
	struct CommonMessage {
		u32 len;
		u64 entityid;
		u16 msgid;
		u16 flags;
		Byte payload[0];
	};
#pragma pack(pop)

	bool sendMessage(Easynet* easynet, SOCKET socket, u64 entityid, u32 msgid, const google::protobuf::Message* message);
	
#if false
#define NEW_MSG(STRUCTURE, ...)	\
		char __message_buffer__[SIZE];\
		STRUCTURE* msg = Constructor((STRUCTURE *)(__message_buffer__), ##__VA_ARGS__);
#endif	
	
#define DECLARE_MESSAGE() \
		typedef bool (*MESSAGE_ROUTINE)(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg);\
		struct MessageTable {\
			MESSAGE_ROUTINE table[MAX_MESSAGE_ID];\
		};\
		static MessageTable __t;\
		__attribute__((constructor)) static void __t_init() {\
			memset(__t.table, 0, sizeof(__t.table));\
		}
		
	
#define ON_MSG(MSGID, STRUCTURE) \
		static bool onMessage_raw_##STRUCTURE(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg);\
		static void onMessage_##STRUCTURE(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg);\
		__attribute__((constructor)) static void __##STRUCTURE() {\
			assert(MSGID >= 0 && MSGID < MAX_MESSAGE_ID);\
			if (__t.table[MSGID]) {\
				fprintf(stderr, "Duplicate message id:%d, %s\n", MSGID, #STRUCTURE);\
				abort();\
			}\
			else {\
				__t.table[MSGID] = onMessage_raw_##STRUCTURE;\
			}\
		}\
		static bool onMessage_raw_##STRUCTURE(Easynet* easynet, SOCKET socket, CommonMessage* rawmsg) {\
			STRUCTURE msg;\
			bool rc = msg.ParseFromArray(rawmsg->payload, rawmsg->len - sizeof(CommonMessage));\
			CHECK_RETURN(rc, false, "%s ParseFromArray failure: %d, %ld", #STRUCTURE, rawmsg->len, sizeof(CommonMessage));\
			onMessage_##STRUCTURE(easynet, socket, &msg, rawmsg);\
			return rc;\
		}\
		static void onMessage_##STRUCTURE(Easynet* easynet, SOCKET socket, STRUCTURE* msg, CommonMessage* rawmsg)
		
	
#define DISPATCH_MESSAGE(easynet, socket, rawmsg) \
		({\
			bool rc = false;\
			if (rawmsg->msgid < 0 || rawmsg->msgid >= MAX_MESSAGE_ID) {\
				Error << "illegal rawmsg->msgid: " << rawmsg->msgid;\
			}\
			else {\
				rc = true;\
				if (__t.table[rawmsg->msgid]) {\
					rc = __t.table[rawmsg->msgid](easynet, socket, rawmsg);\
				}\
				else {\
					Error << "unhandled rawmsg: " << rawmsg->msgid;\
				}\
			}\
			rc;\
		})	
}

#endif

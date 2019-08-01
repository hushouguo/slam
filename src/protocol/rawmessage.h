/*
 * \file: rawmessage.h
 * \brief: Created by hushouguo at 14:51:31 Jul 30 2019
 */
 
#ifndef __RAWMESSAGE_H__
#define __RAWMESSAGE_H__

#pragma pack(push, 1)
	struct rawmessage_s {
		uint32_t len;
		uint64_t entityid;
		msgid_t id;
		uint16_t flags;
		char payload[0];
	};
#pragma pack(pop)

typedef rawmessage_s rawmessage_t;

#endif

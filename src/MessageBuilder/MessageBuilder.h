/*
 * \file: MessageBuilder.h
 * \brief: Created by hushouguo at 14:30:49 Aug 06 2019
 */
 
#ifndef __MESSAGE_BUILDER_H__
#define __MESSAGE_BUILDER_H__
 
typedef int msgid_t;
typedef struct lua_State lua_State;

#define BUILDER_OK      0
#define BUILDER_ERROR   -1

#ifdef __cplusplus
extern "C" {
#endif

    extern void MessageBuilderInit();
    extern void MessageBuilderDestroy();
    
	extern int MessageBuilderLoadDescriptor(const char* filename); // filename also is folder
	extern int MessageBuilderRegister(msgid_t msgid, const char* name);
	
	/* decode buffer to lua's table */
	extern int MessageBuilderDecode(lua_State* L, msgid_t msgid, const void* buf, size_t bufsize);
	/* encode lua's table to buffer */
	extern void* MessageBuilderEncode(lua_State* L, msgid_t msgid, size_t* size);
	
#ifdef __cplusplus	
}
#endif

#endif

/*
 * \file: lua_reg_standard_functions.cpp
 * \brief: Created by hushouguo at 15:44:05 Mar 26 2019
 */

#include "tnode.h"
#include "tools/ByteBuffer.h"
#include "tools/base64.h"
#include "tools/Runnable.h"
#include "tools/Tools.h"
#include "tools/Singleton.h"
#include "json/json_parser.h"
#include "message/ServiceMessage.h"
#include "time/Time.h"
#include "lua/luaT.h"
#include "lua/luaT_reg_functions.h"
#include "lua/luaT_xml_parser.h"
#include "lua/luaT_json_parser.h"
#include "lua/MessageParser.h"
#include "service/Service.h"
#include "service/ServiceManager.h"

BEGIN_NAMESPACE_TNODE {
	//
	// table json_decode(string)
	static int cc_json_decode(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));	
		size_t len = 0;
		const char* jsonstr = lua_tolstring(L, -args, &len);
		return luaT_json_parser_decode(L, jsonstr, len) ? 1 : 0;
	}

	//
	// string json_encode(table)
	static int cc_json_encode(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_istable(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		ByteBuffer byteBuffer;
		if (luaT_json_parser_encode(L, &byteBuffer)) {
			lua_pushstring(L, (const char*)(byteBuffer.rbuffer()));
		}
		else {
			lua_pushnil(L);
		}
		return 1;
	}

	//
	// table xml_decode(string)
	static int cc_xml_decode(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));	
		size_t len = 0;
		const char* file = lua_tolstring(L, -args, &len);	
		return luaT_xml_parser_decode(L, file) ? 1 : 0;
	}

	//
	// string xml_encode(table)
	static int cc_xml_encode(lua_State* L) {
		Error << "xml_encode not implement!";
		return 0;
	}
	
	//
	// int hash_string(string)
	static int cc_hash_string(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));		
		size_t len = 0;
		const char* string = lua_tolstring(L, -args, &len);
		lua_pushinteger(L, hashString(string, len));
		return 1;
	}

	//
	// int random()
	static int cc_random(lua_State* L) {
		lua_pushinteger(L, randomValue());
		return 1;
	}

	//
	// int random_between(int min, int max)
	static int cc_random_between(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));		
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));		
		lua_Integer min = lua_tointeger(L, -args);
		lua_Integer max = lua_tointeger(L, -(args - 1));	
		lua_pushinteger(L, randomBetween(min, max));
		return 1;
	}

	//
	// string base64_encode(string)
	static int cc_base64_encode(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));		
		size_t len = 0;
		const char * string = lua_tolstring(L, -args, &len);	
		std::string base64_string;
	   	base64_encode((unsigned char const*) string, len, base64_string);	
		lua_pushstring(L, base64_string.c_str());
		return 1;
	}

	//
	// string base64_decode(string)
	static int cc_base64_decode(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));			
		std::string encoded_string = lua_tostring(L, -args);	
		std::string decoded_string;
	   	base64_decode(encoded_string, decoded_string);	
		lua_pushstring(L, decoded_string.c_str());
		return 1;
	}
	
	//
	// string message_encode(msgid, table)
	static int cc_message_encode(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_istable(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		u32 msgid = lua_tointeger(L, -args);
		u32 sid = luaT_getOwner(L);
		Service* service = sServiceManager.getService(sid);
		assert(service);
		std::string outstring;
		bool retval = service->messageParser()->encode(L, msgid, outstring);
		CHECK_RETURN(retval, 0, "encode message: %d error", msgid);
		lua_pushstring(L, outstring.c_str());
		return 1;
	}

	//
	// table message_decode(msgid, string)
	static int cc_message_decode(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));			
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));			
		u32 msgid = lua_tointeger(L, -args);
		std::string instring = lua_tostring(L, -(args - 1));
		u32 sid = luaT_getOwner(L);
		Service* service = sServiceManager.getService(sid);
		assert(service);
		bool retval = service->messageParser()->decode(L, msgid, instring);
		CHECK_RETURN(retval, 0, "decode message: %d error", msgid);
		// table is in the top of stack
		return 1;
	}

	//
	// table md5(string)
	static int cc_md5(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));	
		size_t len = 0;
		const char * content = lua_tolstring(L, -args, &len);
		unsigned char digest[MD5_DIGEST_LENGTH];	
		MD5((const unsigned char *)content, len, digest);	
		lua_newtable(L);
		for (int n = 0; n < MD5_DIGEST_LENGTH; ++n) {
			lua_pushinteger(L, n);
			lua_pushinteger(L, digest[n]);
			lua_settable(L, -3);
		}	
		return 1;
	}

	//
	// u64 timesec()
	static int cc_timesec(lua_State* L) {
		sTime.now();
		lua_pushinteger(L, sTime.seconds());
		return 1;
	}

	//
	// u64 timemsec()
	static int cc_timemsec(lua_State* L) {
		sTime.now();
		lua_pushinteger(L, sTime.milliseconds());
		return 1;
	}

	//
	// YYYY, MM, DD, HH, MM, SS timespec([u64])
	static int cc_timespec(lua_State* L) {
		std::time_t seconds;
		std::time_t ts_milliseconds;
	
		int args = lua_gettop(L);
		if (args == 0) {
			auto time_now = std::chrono::system_clock::now();
			auto duration_millisecond = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
			auto millisecond_part = duration_millisecond - std::chrono::duration_cast<std::chrono::seconds>(duration_millisecond);
			seconds = std::chrono::system_clock::to_time_t(time_now);
			ts_milliseconds = millisecond_part.count();
		}
		else if (args == 1) {
			CHECK_RETURN(lua_isnumber(L, -1), 0, "[%s]", lua_typename(L, lua_type(L, -1)));		
			lua_Integer milliseconds = lua_tointeger(L, -1);
			seconds = milliseconds / 1000;
			ts_milliseconds = milliseconds % 1000;
		}
	
		std::tm * local_time_now = std::localtime(&seconds);
	
		lua_pushinteger(L, local_time_now->tm_year + 1900);
		lua_pushinteger(L, local_time_now->tm_mon + 1);
		lua_pushinteger(L, local_time_now->tm_mday);
		lua_pushinteger(L, local_time_now->tm_hour);
		lua_pushinteger(L, local_time_now->tm_min);
		lua_pushinteger(L, local_time_now->tm_sec);
		lua_pushinteger(L, ts_milliseconds);
	
		return 7;
	}

	//
	// string timestamp()
	// string timestamp(seconds)
	// string timestamp(string)
	// string timestamp(seconds, string)
	static int cc_timestamp(lua_State* L) {
		char buffer[64];
		int args = lua_gettop(L);
		
		if (args == 0) {
			timestamp(buffer, sizeof(buffer));
		}
		else if (args == 1) {
			if (lua_isnumber(L, -1)) { // string timestamp(seconds)
				timestamp(buffer, sizeof(buffer), lua_tointeger(L, -1));
			}
			else if (lua_isstring(L, -1)) { // string timestamp(string)
				timestamp(buffer, sizeof(buffer), 0, lua_tostring(L, -1));
			}
			else {
				Error.cout("`%s` parameter error:%s", __FUNCTION__, lua_typename(L, lua_type(L, -1)));
				return 0;
			}
		}
		else if (args == 2) {
			if (!lua_isnumber(L, -args)) {
				Error.cout("`%s` parameter error:%s", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
				return 0;
			}
			if (!lua_isstring(L, -(args - 1))) {
				Error.cout("`%s` parameter error:%s", __FUNCTION__, lua_typename(L, lua_type(L, -(args - 1))));
				return 0;
			}
			timestamp(buffer, sizeof(buffer), lua_tointeger(L, -args), lua_tostring(L, -(args - 1)));
		}
		else {
			Error.cout("`%s` parameter error:%d", __FUNCTION__, args);
		}
	
		lua_pushstring(L, buffer);
	
		return 1;
	}

	//
	// void msleep(milliseconds)
	static int cc_msleep(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		lua_Integer milliseconds = lua_tointeger(L, -1);	
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));	
		return 0;
	}

	//
	// logger
	static int cc_log_debug(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* content = lua_tostring(L, -args);
		if (content) {
			Debug << "[LUA] " << content;
		}
		return 0;
	}
	static int cc_log_trace(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* content = lua_tostring(L, -args);
		if (content) {
			Trace << "[LUA] " << content;
		}
		return 0;
	}
	static int cc_log_alarm(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* content = lua_tostring(L, -args);
		if (content) {
			Alarm << "[LUA] " << content;
		}
		return 0;
	}
	static int cc_log_error(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* content = lua_tostring(L, -args);
		if (content) {
			lua_Debug ar;
			lua_getstack(L, 1, &ar);
			lua_getinfo(L, "nSl", &ar);
			Error << "[LUA] (" << ar.source << ":" << ar.currentline << ") " << content;
		}
		return 0;
	}

	//
	// bool newservice(entryfile)
	static int cc_newservice(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args: %d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* entryfile = lua_tostring(L, -args);
		bool retval = sServiceManager.newservice(entryfile);
		lua_pushboolean(L, retval ? 1 : 0);
		return 1;
	}

	//
	// void exitservice()
	static int cc_exitservice(lua_State* L) {
		u32 sid = luaT_getOwner(L);
		Service* service = sServiceManager.getService(sid);
		assert(service);
		sServiceManager.exitservice(sid);
		return 0;
	}
	

	//
	// fd newserver(address, port)
	static int cc_newserver(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args: %d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* address = lua_tostring(L, -args);
		int port = lua_tointeger(L, -(args - 1));		
		//TODO:SOCKET fd = sNetworkManager.newserver(address, port);
		SOCKET fd = -1;
		if (fd == -1) {
			Error << "newserver: " << address << ", port: " << port << " failure";
		}		
		lua_pushinteger(L, fd);
		return 1;
	}


	//
	// fd newclient(address, port)
	static int cc_newclient(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args: %d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* address = lua_tostring(L, -args);
		int port = lua_tonumber(L, -(args - 1));		
		//TODO:SOCKET fd = sNetworkManager.newclient(address, port);
		SOCKET fd = -1;
		if (fd == -1) {
			Error << "newclient: " << address << ", port: " << port << " failure";
		}		
		lua_pushinteger(L, fd);
		return 1;
	}

	//
	// void response(fd, entityid, msgid, o)
	static int cc_response(lua_State* L) {
#if 0		
		int args = lua_gettop(L);
		CHECK_RETURN(args == 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		CHECK_RETURN(lua_isnumber(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		CHECK_RETURN(lua_istable(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
		
		SOCKET fd = lua_tointeger(L, -args);
		u64 entityid = lua_tointeger(L, -(args - 1));
		u32 msgid = lua_tointeger(L, -(args - 2));

		u32 sid = luaT_getOwner(L);
		Service* service = sServiceManager.getService(sid);
		assert(service);

		google::protobuf::Message* message = service->messageParser()->encode(L, msgid);
		CHECK_RETURN(message, 0, "encode message: %d error", msgid);

		size_t byteSize = message->ByteSize();		
		//TODO:ServiceMessage* msg = allocate_message(byteSize);
		ServiceMessage* msg = nullptr;

		bool rc = message->SerializeToArray(msg->rawmsg.payload, byteSize);
		if (!rc) {
			release_message(msg);
			Error.cout("Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
			return 0;
		}
		
		msg->fd = fd;
		msg->rawmsg.entityid = entityid;
		msg->rawmsg.msgid = msgid;
		msg->rawmsg.flags = 0;
		msg->rawmsg.len = byteSize + sizeof(Socketmessage);
		
		//TODO:sNetworkManager.sendMessage(msg);
#endif		
		return 0;
	}

	//
	// void loadmsg(filename) // filename also is a directory
	static int cc_loadmsg(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));

		u32 sid = luaT_getOwner(L);
		Service* service = sServiceManager.getService(sid);
		assert(service);
		
		const char* filename = lua_tostring(L, -args);
		bool rc = service->messageParser()->loadmsg(filename);
		CHECK_RETURN(rc, 0, "loadmsg: %s failure", filename);
		return 0;
	}
	
	//
	// void regmsg(msgid, name)
	static int cc_regmsg(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));

		u32 sid = luaT_getOwner(L);
		Service* service = sServiceManager.getService(sid);
		assert(service);
		
		u32 msgid = lua_tointeger(L, -args);
		const char* name = lua_tostring(L, -(args - 1));
		bool retval = service->messageParser()->regmsg(msgid, name);
		CHECK_RETURN(retval, 0, "regmsg: %d, %s failure", msgid, name);
		return 0;
	}

	//
	// void release_message(ServiceMessage*)
	static int cc_release_message(lua_State* L) {
#if 0		
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_islightuserdata(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		void* userdata = lua_touserdata(L, -args);
		release_message(static_cast<ServiceMessage*>(userdata));
#endif		
		return 0;
	}

	//
	// void closesocket(fd)
	static int cc_closesocket(lua_State* L) {
#if 0		
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		SOCKET fd = lua_tointeger(L, -args);
		sNetworkManager.closeSocket(fd, "LUA");
#endif		
		return 0;
	}
	
	//
	// void newtimer(milliseconds, function(milliseconds) end)
	static int cc_newtimer(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isfunction(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));		

		u32 sid = luaT_getOwner(L);
		Service* service = sServiceManager.getService(sid);
		assert(service);
		
		lua_Integer milliseconds = lua_tointeger(L, -1);
		lua_pushvalue(L, -(args - 1));
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		//TODO:
		(void)(milliseconds);
		(void)(ref);
		
		return 0;
	}
	
	void luaT_reg_functions(lua_State* L) {
		luaT_beginNamespace(L, LUA_REGISTER_NAMESPACE);

		// service
		LUA_REGISTER(L, "newservice", cc_newservice);
		LUA_REGISTER(L, "exitservice", cc_exitservice);

		// network
		LUA_REGISTER(L, "newserver", cc_newserver);
		LUA_REGISTER(L, "newclient", cc_newclient);
		LUA_REGISTER(L, "response", cc_response);
		LUA_REGISTER(L, "release_message", cc_release_message);
		LUA_REGISTER(L, "loadmsg", cc_loadmsg);
		LUA_REGISTER(L, "regmsg", cc_regmsg);
		LUA_REGISTER(L, "closesocket", cc_closesocket);

		// logger
		LUA_REGISTER(L, "log_debug", cc_log_debug);
		LUA_REGISTER(L, "log_trace", cc_log_trace);
		LUA_REGISTER(L, "log_alarm", cc_log_alarm);
		LUA_REGISTER(L, "log_error", cc_log_error);
		
		// json
		LUA_REGISTER(L, "json_decode", cc_json_decode);
		LUA_REGISTER(L, "json_encode", cc_json_encode);

		// xml
		LUA_REGISTER(L, "xml_decode", cc_xml_decode);
		LUA_REGISTER(L, "xml_encode", cc_xml_encode);

		// hash string
		LUA_REGISTER(L, "hash_string", cc_hash_string);
		
		// random
		LUA_REGISTER(L, "random", cc_random);
		LUA_REGISTER(L, "random_between", cc_random_between);

		// base64
		LUA_REGISTER(L, "base64_encode", cc_base64_encode);
		LUA_REGISTER(L, "base64_decode", cc_base64_decode);

		// md5
		LUA_REGISTER(L, "md5", cc_md5);

		// time
		LUA_REGISTER(L, "timesec", cc_timesec);
		LUA_REGISTER(L, "timemsec", cc_timemsec);
		LUA_REGISTER(L, "timestamp", cc_timestamp);
		LUA_REGISTER(L, "timespec", cc_timespec);
		LUA_REGISTER(L, "msleep", cc_msleep);

		// message parser
		LUA_REGISTER(L, "message_encode", cc_message_encode);
		LUA_REGISTER(L, "message_decode", cc_message_decode);
		
		// timer
		LUA_REGISTER(L, "newtimer", cc_newtimer);
		
		//TODO: file i/o
		//TODO: http
		//TODO: curl
		//TODO: db agent
		
		luaT_endNamespace(L);
	}
}

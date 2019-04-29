/*
 * \file: lua_reg_standard_functions.cpp
 * \brief: Created by hushouguo at 15:44:05 Mar 26 2019
 */

#include "tnode.h"
#include "tools/ByteBuffer.h"
#include "tools/base64.h"
#include "tools/Entry.h"
#include "tools/Singleton.h"
#include "tools/Tools.h"
#include "tools/Spinlocker.h"
#include "tools/LockfreeQueue.h"
#include "tools/ThreadPool.h"
#include "tools/Runnable.h"
#include "tools/Registry.h"
#include "json/json_parser.h"
#include "message/ServiceMessage.h"
#include "message/MessageParser.h"
#include "config/Config.h"
#include "lua/luaT.h"
#include "time/Time.h"
#include "time/Timer.h"
#include "time/TimerManager.h"
#include "lua/luaT_reg_functions.h"
#include "lua/luaT_xml_parser.h"
#include "lua/luaT_json_parser.h"
#include "lua/luaT_message_parser.h"
#include "db/Easydb.h"
#include "service/Service.h"
#include "service/ServiceManager.h"
#include "net/NetworkManager.h"


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
	// string random_string(length, with_digit=true, with_lower=true, with_capital=true
	static int cc_random_string(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args > 0, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		lua_Integer len = lua_tointeger(L, -args);
		bool with_digit = true;
		if (args > 1) {
			CHECK_RETURN(lua_isboolean(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
			with_digit = lua_toboolean(L, -(args - 1));
		}
		bool with_lower = true;
		if (args > 2) {
			CHECK_RETURN(lua_isboolean(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
			with_lower = lua_toboolean(L, -(args - 2));
		}
		bool with_capital = true;
		if (args > 3) {
			CHECK_RETURN(lua_isboolean(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
			with_capital = lua_toboolean(L, -(args - 3));
		}
		std::string result;
		randomString(result, len, with_digit, with_lower, with_capital);
		lua_pushlstring(L, result.data(), result.length());
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
		std::string outstring;
		bool rc = luaT_message_parser_encode(sServiceManager.getService(L)->msgParser(), L, msgid, outstring);
		CHECK_RETURN(rc, 0, "encode message: %d error", msgid);
		lua_pushlstring(L, outstring.data(), outstring.length());
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
		size_t len = 0;
		const char* instring = lua_tolstring(L, -(args - 1), &len);
		bool rc = luaT_message_parser_decode(sServiceManager.getService(L)->msgParser(), L, msgid, instring, len);
		CHECK_RETURN(rc, 0, "decode message: %d error", msgid);		
		return 1; // table is in the top of stack
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
				Error("`%s` parameter error:%s", __FUNCTION__, lua_typename(L, lua_type(L, -1)));
				return 0;
			}
		}
		else if (args == 2) {
			if (!lua_isnumber(L, -args)) {
				Error("`%s` parameter error:%s", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
				return 0;
			}
			if (!lua_isstring(L, -(args - 1))) {
				Error("`%s` parameter error:%s", __FUNCTION__, lua_typename(L, lua_type(L, -(args - 1))));
				return 0;
			}
			timestamp(buffer, sizeof(buffer), lua_tointeger(L, -args), lua_tostring(L, -(args - 1)));
		}
		else {
			Error("`%s` parameter error:%d", __FUNCTION__, args);
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
	// u32 newservice(entryfile)
	static int cc_newservice(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args: %d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* entryfile = lua_tostring(L, -args);
		Service* service = sServiceManager.newservice(entryfile);
		lua_pushinteger(L, service ? service->id : -1);
		return 1;
	}

	//
	// void exitservice()
	static int cc_exitservice(lua_State* L) {
		u32 sid = luaT_getOwner(L);
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
		SOCKET fd = sNetworkManager.easynet()->createServer(address, port);
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
		int port = lua_tointeger(L, -(args - 1));
		SOCKET fd = sNetworkManager.easynet()->createClient(address, port);
		if (fd == -1) {
			Error << "newclient: " << address << ", port: " << port << " failure";
		}		
		lua_pushinteger(L, fd);
		return 1;
	}

	//
	// fd getsocket(name)
	static int cc_getsocket(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args: %d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* name = lua_tostring(L, -args);
		SOCKET fd = sNetworkManager.findSocket(name);
		lua_pushinteger(L, fd);
		return 1;
	}


	//
	// bool response(fd, entityid, msgid, o)
	static int cc_response(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		CHECK_RETURN(lua_isnumber(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		CHECK_RETURN(lua_istable(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
		
		SOCKET fd = lua_tointeger(L, -args);
		u64 entityid = lua_tointeger(L, -(args - 1));
		u32 msgid = lua_tointeger(L, -(args - 2));

		//
		// encode table of lua to protobuf::Message
		Message* message = luaT_message_parser_encode(sServiceManager.getService(L)->msgParser(), L, msgid);
		CHECK_RETURN(message, 0, "encode message: %d error", msgid);

		//
		// allocate new ServiceMessage
		size_t byteSize = message->ByteSize();
		const void* netmsg = sNetworkManager.easynet()->allocateMessage(byteSize + sizeof(ServiceMessage));
		size_t len = 0;
		ServiceMessage* msg = (ServiceMessage*) sNetworkManager.easynet()->getMessageContent(netmsg, &len); 
		assert(len == byteSize + sizeof(ServiceMessage));
		
		//
		// serialize protobuf::Message to ServiceMessage
		bool rc = message->SerializeToArray(msg->payload, byteSize);
		if (!rc) {
			sNetworkManager.easynet()->releaseMessage(netmsg);
			Error("Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
			return 0;
		}

		//
		// send ServiceMessage to network
		msg->len = byteSize + sizeof(ServiceMessage);
		msg->entityid = entityid;
		msg->msgid = msgid;
		msg->flags = 0;
		rc = sNetworkManager.easynet()->sendMessage(fd, netmsg);

		lua_pushboolean(L, rc);
		return 1;
	}

	//
	// bool loadmsg(filename) // filename also is a directory
	static int cc_loadmsg(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* filename = lua_tostring(L, -args);
		bool rc = sServiceManager.getService(L)->msgParser()->LoadMessageDescriptor(filename);
		lua_pushboolean(L, rc);
		return 1;
	}
	
	//
	// bool regmsg(msgid, name)
	static int cc_regmsg(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		u32 msgid = lua_tointeger(L, -args);
		const char* name = lua_tostring(L, -(args - 1));
		bool rc = sServiceManager.getService(L)->msgParser()->RegisteMessage(msgid, name);
		lua_pushboolean(L, rc);
		return 1;
	}

	//
	// void closesocket(fd)
	static int cc_closesocket(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		SOCKET fd = lua_tointeger(L, -args);
		sNetworkManager.easynet()->closeSocket(fd);
		return 0;
	}
	
	//
	// u32 newtimer(milliseconds, times, ctx, function(id, ctx) end)
	// times: <= 0: forever, > 0: special times
	static int cc_newtimer(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		lua_Integer milliseconds = lua_tointeger(L, -args);
		lua_Integer times = lua_tointeger(L, -(args - 1));
		
		luaT_Value ctx;
		if (lua_isboolean(L, -(args - 2))) {
			ctx = lua_toboolean(L, -(args - 2)) != 0;
		}
		else if (lua_isnumber(L, -(args - 2))) {
			lua_Number value = lua_tonumber(L, -(args - 2));
			if (isInteger(value)) {
				ctx = lua_tointeger(L, -(args - 2));
			}
			else {
				ctx = lua_tonumber(L, -(args - 2));
			}
		}
		else if (lua_isstring(L, -(args - 2))) {
			ctx = lua_tostring(L, -(args - 2));
		}
		else if (lua_isnil(L, -(args - 2))) {
			// ctx is nil
		}
		else {
			Error << "Not support ctx type: " << lua_typename(L, lua_type(L, -(args - 2)));
		}
		
		CHECK_RETURN(lua_isfunction(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));				
		lua_pushvalue(L, -(args - 3));
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		u32 timer_id = sServiceManager.getService(L)->timerManager().createTimer(milliseconds, times, ref, ctx);

		lua_pushinteger(L, timer_id);
		return 1;
	}

	//
	// void set_timer_interval(u32, milliseconds)
	static int cc_set_timer_interval(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		lua_Integer timerid = lua_tointeger(L, -args);
		lua_Integer milliseconds = lua_tointeger(L, -(args - 1));
		sServiceManager.getService(L)->timerManager().setTimerInterval(timerid, milliseconds);
		return 0;
	}
	
	//
	// void set_timer_times(u32, times)
	static int cc_set_timer_times(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		lua_Integer timerid = lua_tointeger(L, -args);
		lua_Integer times = lua_tointeger(L, -(args - 1));
		sServiceManager.getService(L)->timerManager().setTimerTimes(timerid, times);
		return 0;
	}
	
	//
	// void remove_timer(u32)
	static int cc_remove_timer(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isnumber(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		lua_Integer timerid = lua_tointeger(L, -args);
		sServiceManager.getService(L)->timerManager().removeTimer(timerid);
		return 0;
	}


	//
	// db
	//

	//
	// userdata newdb(host, user, passwd, port, [database])
	static int cc_newdb(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args >= 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		CHECK_RETURN(lua_isstring(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		CHECK_RETURN(lua_isnumber(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
		const char* database = nullptr;
		if (args > 4) {
			CHECK_RETURN(lua_isstring(L, -(args - 4)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 4))));
			database = lua_tostring(L, -(args - 4));
		}
		
		const char* host = lua_tostring(L, -args);
		const char* user = lua_tostring(L, -(args - 1));
		const char* passwd = lua_tostring(L, -(args - 2));
		int port = lua_tointeger(L, -(args - 3));

		//
		// create new Easydb
		Easydb* easydb = Easydb::createInstance();
		assert(easydb);
		bool rc = easydb->connectServer(host, user, passwd, port);
		if (!rc) {
			SafeDelete(easydb);
			CHECK_RETURN(rc, 0, "`newdb` failure, connectServer");
		}

		//
		// connect database if user special database
		if (database) {
			rc = easydb->selectDatabase(database);
			if (!rc) {
				SafeDelete(easydb);
				CHECK_RETURN(rc, 0, "`newdb` failure, selectDatabase");
			}
		}

		//
		// create userdata and setup metatable into userdata
		Easydb** db = (Easydb**) lua_newuserdata(L, sizeof(Easydb*));
		*db = easydb;
		luaL_getmetatable(L, LUA_METATABLE_DB_NAME);
		lua_setmetatable(L, -2);
		
		return 1;
	}

	//----------------------------------------------------------------------------------------------------

	//
	// bool db:create_database(database)
	static int cc_db_create_database(lua_State* L) {	
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
	
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* database = lua_tostring(L, -(args - 1));
		bool rc = (*db)->createDatabase(database);
		
		lua_pushboolean(L, rc);
		return 1;
	}
	
	//
	// bool db:select_database(database)
	static int cc_db_select_database(lua_State* L) {	
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
	
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* database = lua_tostring(L, -(args - 1));
		bool rc = (*db)->selectDatabase(database);
		
		lua_pushboolean(L, rc);
		return 1;
	}

	//
	// bool db:loadmsg(filename) // filename also is a directory
	static int cc_db_loadmsg(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
		
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* filename = lua_tostring(L, -(args - 1));
		bool rc = (*db)->tableParser()->LoadMessageDescriptor(filename);
		lua_pushboolean(L, rc);
		return 1;
	}
	
	//
	// bool db:regtable(table, name) // table => name of protobuf::Message
	static int cc_db_regtable(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
		
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		CHECK_RETURN(lua_isstring(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		size_t len = 0;
		const char* table = lua_tolstring(L, -(args - 1), &len);
		const char* name = lua_tostring(L, -(args - 2));
		u32 msgid = hashString(table, len);
		bool rc = (*db)->createTable(table);
		if (rc) {
			rc = (*db)->tableParser()->RegisteMessage(msgid, name);
		}
		lua_pushboolean(L, rc);
		return 1;
	}
	
	//
	// u64 db:create_object(table, [u64], o)
	static int cc_db_create_object(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args >= 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);		
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
	
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		size_t len = 0;
		const char* table = lua_tolstring(L, -(args - 1), &len);
		u32 msgid = hashString(table, len);
		u64 id = 0;
		if (args > 3) {
			CHECK_RETURN(lua_isnumber(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
			id = lua_tointeger(L, -(args - 2));
			CHECK_RETURN(lua_istable(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
		}
		else {
			CHECK_RETURN(lua_istable(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		}
		
		//
		// encode lua's table to Message		
		Message* message = luaT_message_parser_encode((*db)->tableParser(), L, msgid);
		CHECK_RETURN(message, 0, "encode message: %d, %s error", msgid, table);

		//
		// insert object to db
		u64 autoid = (*db)->createObject(table, id, message);
		if (autoid == 0) {
			CHECK_RETURN(false, 0, "createObject failure, id: %ld, msgid:%d, table:%s", id, msgid, table);
		}

		lua_pushinteger(L, autoid);
		return 1;
	}
	
	//
	// bool db:delete_object(table, uint64_t)
	static int cc_db_delete_object(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);		
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
	
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		CHECK_RETURN(lua_isnumber(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		const char* table = lua_tostring(L, -(args - 1));
		u64 id = lua_tointeger(L, -(args - 2));

		//
		// delete object from db
		bool rc = (*db)->deleteObject(table, id);
		lua_pushboolean(L, rc);
		return 1;
	}

	//
	// bool db:serialize(table, uint64_t, o)
	static int cc_db_serialize(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);		
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
	
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		CHECK_RETURN(lua_isnumber(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		CHECK_RETURN(lua_istable(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
		size_t len = 0;
		const char* table = lua_tolstring(L, -(args - 1), &len);
		u32 msgid = hashString(table, len);
		u64 id = lua_tointeger(L, -(args - 2));

		//
		// encode table of lua to protobuf::Message
		Message* message = luaT_message_parser_encode((*db)->tableParser(), L, msgid);
		CHECK_RETURN(message, 0, "encode message: %d, %s error", msgid, table);

		//
		// update object to db
		bool rc = (*db)->updateObject(table, id, message);
		if (!rc) {
			CHECK_RETURN(false, 0, "updateObject failure, id: %ld, msgid:%d, table:%s", id, msgid, table);
		}
		
		lua_pushboolean(L, rc);
		return 1;
	}

	//
	// bool db:flush_object(table, uint64_t)
	static int cc_db_flush_object(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);		
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
	
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		CHECK_RETURN(lua_isnumber(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		size_t len = 0;
		const char* table = lua_tolstring(L, -(args - 1), &len);
		u64 id = lua_tointeger(L, -(args - 2));

		//
		// flush object to db
		bool rc = (*db)->flushObject(table, id);
		CHECK_ERROR(rc, "flushObject failure, id: %ld, table:%s", id, table);
		
		lua_pushboolean(L, rc);
		return 1;
	}


	//
	// o db:unserialize(table, uint64_t)
	static int cc_db_unserialize(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);		
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
	
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		CHECK_RETURN(lua_isnumber(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
		size_t len = 0;
		const char* table = lua_tolstring(L, -(args - 1), &len);
		u64 id = lua_tointeger(L, -(args - 2));

		u32 msgid = hashString(table, len);

		//
		// fetch ByteBuffer from db
		ByteBuffer* buffer = (*db)->retrieveObject(table, id);
		CHECK_RETURN(buffer, 0, "retrieveObject: %ld from table: %s failure", id, table);

		//
		// decode protobuf::Message to lua
		bool rc = luaT_message_parser_decode((*db)->tableParser(), L, msgid, buffer->rbuffer(), buffer->size());
		CHECK_RETURN(rc, 0, "decode object: %ld, table: %s to lua failure", id, table);
				
		return 1;
	}
	
	static int __cc_db_gc(lua_State *L) {
		Easydb** db = (Easydb**) luaL_checkudata(L, 1, LUA_METATABLE_DB_NAME);		
		luaL_argcheck(L, db != NULL, 1, "invalid `db` userdata");
		if (db) {
			Debug("__db_gc: %p", *db);
			// destroy Easydb from this service
			SafeDelete(*db);
		}
		return 0;
	}
	
	static int __cc_db_tostring(lua_State *L) {
		lua_pushstring(L, "db");
		return 1;
	}

	//
	// logger
	//

	//
	// cc.log_XXXXX to syslog
	static int cc_syslog_debug(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* content = lua_tostring(L, -args);
		if (content) {
			Debug << "[LUA] " << content;
		}
		return 0;
	}
	static int cc_syslog_trace(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* content = lua_tostring(L, -args);
		if (content) {
			Trace << "[LUA] " << content;
		}
		return 0;
	}
	static int cc_syslog_alarm(lua_State* L) {
		int args = lua_gettop(L);
		CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
		const char* content = lua_tostring(L, -args);
		if (content) {
			Alarm << "[LUA] " << content;
		}
		return 0;
	}
	static int cc_syslog_error(lua_State* L) {
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
	// userdata newlog()
	static int cc_newlog(lua_State* L) {
		//
		// create new Easylog
		logger::Easylog* easylog = logger::EasylogCreator::create();
		assert(easylog);

		//
		// create userdata and setup metatable into userdata
		logger::Easylog** log = (logger::Easylog**) lua_newuserdata(L, sizeof(logger::Easylog*));
		*log = easylog;
		luaL_getmetatable(L, LUA_METATABLE_LOGGER_NAME);
		lua_setmetatable(L, -2);
		
		return 1;
	}

	//
	// string log:destination([string dir])
	static int cc_log_destination(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 1 || args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
		
		const char* dir = nullptr;
		if (args == 2) {
			CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
			dir = lua_tostring(L, -(args - 1));
		}
		if (dir) {
			(*log)->set_destination(dir);
		}
		lua_pushstring(L, (*log)->destination());
		return 1;
	}
	
	//
	// bool log:toserver(int level, [string address, int port])
	static int cc_log_toserver(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2 || args == 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
		
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		EasylogSeverityLevel level = (EasylogSeverityLevel) lua_tointeger(L, -(args - 1));
		const char* address = nullptr;
		int port = 0;
		if (args == 4) {
			CHECK_RETURN(lua_isstring(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
			CHECK_RETURN(lua_isnumber(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
			address = lua_tostring(L, -(args - 2));
			port = lua_tointeger(L, -(args - 3));
			(*log)->set_toserver(level, address, port);
		}
		bool rc = (*log)->toserver(level);
		lua_pushboolean(L, rc);
		return 1;
	}
	
	//
	// bool log:tostdout(int level, [bool])
	static int cc_log_tostdout(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2 || args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
		
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		EasylogSeverityLevel level = (EasylogSeverityLevel) lua_tointeger(L, -(args - 1));
		if (args == 3) {
			CHECK_RETURN(lua_isboolean(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
			bool enable = lua_toboolean(L, -(args - 2));
			(*log)->set_tostdout(level, enable);
		}
		lua_pushboolean(L, (*log)->tostdout(level));
		return 1;
	}
	
	//
	// string log:tofile(int level, [string filename])
	static int cc_log_tofile(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2 || args == 3, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
		
		CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		EasylogSeverityLevel level = (EasylogSeverityLevel) lua_tointeger(L, -(args - 1));
		if (args == 3) {
			CHECK_RETURN(lua_isstring(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
			const char* filename = lua_tostring(L, -(args - 2));
			(*log)->set_tofile(level, filename);
		}
		lua_pushstring(L, (*log)->tofile(level));
		return 1;
	}
	
	//
	// bool log:autosplit_day([bool])
	static int cc_log_autosplit_day(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 1 || args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");

		if (args == 2) {
			CHECK_RETURN(lua_isboolean(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
			bool value = lua_toboolean(L, -(args - 1));
			(*log)->set_autosplit_day(value);
		}
		bool rc = (*log)->autosplit_day();
		lua_pushboolean(L, rc);
		return 1;
	}
	
	//
	// bool log:autosplit_hour([bool])
	static int cc_log_autosplit_hour(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 1 || args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");

		if (args == 2) {
			CHECK_RETURN(lua_isboolean(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
			bool value = lua_toboolean(L, -(args - 1));
			(*log)->set_autosplit_hour(value);
		}
		bool rc = (*log)->autosplit_hour();
		lua_pushboolean(L, rc);
		return 1;
	}
	
	//
	// int log:level([int level])
	//		GLOBAL			=	0,
	//		LEVEL_DEBUG 	=	1,
	//		LEVEL_TRACE 	=	2,
	//		LEVEL_ALARM 	=	3,
	//		LEVEL_ERROR 	=	4,
	static int cc_log_level(lua_State* L) {
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 1 || args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");

		if (args == 2) {
			CHECK_RETURN(lua_isnumber(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
			EasylogSeverityLevel level = (EasylogSeverityLevel) lua_tointeger(L, -(args - 1));
			(*log)->set_level(level);
		}
		lua_pushinteger(L, (*log)->level());
		return 1;
	}

	//----------------------------------------------------------------------------------------------------

	//
	// void db:debug(string)
	static int cc_log_debug(lua_State* L) {
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
	
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* s = lua_tostring(L, -(args - 1));
		if (s) {
			logger::EasylogMessage(*log, logger::LEVEL_DEBUG,  __FILE__, __LINE__, __FUNCTION__) << s;
		}
		return 0;
	}

	//
	// void db:trace(string)
	static int cc_log_trace(lua_State* L) {
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);		
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
	
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* s = lua_tostring(L, -(args - 1));
		if (s) {
			logger::EasylogMessage(*log, logger::LEVEL_TRACE,  __FILE__, __LINE__, __FUNCTION__) << s;
		}
		return 0;
	}
	
	//
	// void db:alarm(string)
	static int cc_log_alarm(lua_State* L) {
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);		
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
	
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* s = lua_tostring(L, -(args - 1));
		if (s) {
			logger::EasylogMessage(*log, logger::LEVEL_ALARM,  __FILE__, __LINE__, __FUNCTION__) << s;
		}
		return 0;
	}

	//
	// void db:error(string)	
	static int cc_log_error(lua_State* L) {
		logger::Easylog** log = (logger::Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);		
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
	
		int args = lua_gettop(L);// this:userdata in stack bottom
		CHECK_RETURN(args == 2, 0, "`%s` lack args:%d", __FUNCTION__, args);
		CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
		const char* s = lua_tostring(L, -(args - 1));
		if (s) {
			lua_Debug ar;
			lua_getstack(L, 1, &ar);
			lua_getinfo(L, "nSl", &ar);
			//Error << "[LUA] (" << ar.source << ":" << ar.currentline << ") " << content;
			logger::EasylogMessage(*log, logger::LEVEL_ERROR,  __FILE__, __LINE__, __FUNCTION__) 
				<< "(" << ar.source << ":" << ar.currentline << ") " << s;
		}
		return 0;
	}

	
	static int __cc_log_gc(lua_State *L) {		
		Easylog** log = (Easylog**) luaL_checkudata(L, 1, LUA_METATABLE_LOGGER_NAME);		
		luaL_argcheck(L, log != NULL, 1, "invalid `log` userdata");
		if (log) {
			Debug("__log_gc: %p", *log);
			// destroy Easylog from this service
			SafeDelete(*log);
		}
		return 0;
	}
	
	static int __cc_log_tostring(lua_State *L) {
		lua_pushstring(L, "log");
		return 1;
	}

	
	void luaT_reg_functions(lua_State* L) {
		luaT_beginNamespace(L, LUA_REGISTER_NAMESPACE);

		//
		// u32 newservice(entryfile)
		LUA_REGISTER(L, "newservice", cc_newservice);
		//
		// void exitservice()
		LUA_REGISTER(L, "exitservice", cc_exitservice);

		//
		// fd newserver(address, port)
		LUA_REGISTER(L, "newserver", cc_newserver);
		//
		// fd newclient(address, port)
		LUA_REGISTER(L, "newclient", cc_newclient);
		//
		// bool response(fd, entityid, msgid, o)
		LUA_REGISTER(L, "response", cc_response);
		//
		// bool loadmsg(filename) // filename also is a directory
		LUA_REGISTER(L, "loadmsg", cc_loadmsg);
		//
		// bool regmsg(msgid, name)
		LUA_REGISTER(L, "regmsg", cc_regmsg);
		//
		// void closesocket(fd)
		LUA_REGISTER(L, "closesocket", cc_closesocket);
		//
		// fd getsocket(name)
		LUA_REGISTER(L, "getsocket", cc_getsocket);

		//
		// table json_decode(string)
		LUA_REGISTER(L, "json_decode", cc_json_decode);
		//
		// string json_encode(table)
		LUA_REGISTER(L, "json_encode", cc_json_encode);

		//
		// table xml_decode(string)
		LUA_REGISTER(L, "xml_decode", cc_xml_decode);
		//
		// string xml_encode(table)
		LUA_REGISTER(L, "xml_encode", cc_xml_encode);

		// 
		// int hash_string(string)
		LUA_REGISTER(L, "hash_string", cc_hash_string);

		//
		// int random()
		LUA_REGISTER(L, "random", cc_random);
		//
		// int random_between(int min, int max)
		LUA_REGISTER(L, "random_between", cc_random_between);
		//
		// string random_string(length, with_digit=true, with_lower=true, with_capital=true
		LUA_REGISTER(L, "random_string", cc_random_string);

		//
		// string base64_encode(string)
		LUA_REGISTER(L, "base64_encode", cc_base64_encode);
		//
		// string base64_decode(string)
		LUA_REGISTER(L, "base64_decode", cc_base64_decode);

		//
		// table md5(string)
		LUA_REGISTER(L, "md5", cc_md5);

		//
		// u64 timesec()
		LUA_REGISTER(L, "timesec", cc_timesec);
		//
		// u64 timemsec() 
		LUA_REGISTER(L, "timemsec", cc_timemsec);
		//
		// string timestamp()
		// string timestamp(seconds)
		// string timestamp(string)
		// string timestamp(seconds, string)
		LUA_REGISTER(L, "timestamp", cc_timestamp);
		// YYYY, MM, DD, HH, MM, SS timespec([u64])
		LUA_REGISTER(L, "timespec", cc_timespec);
		// void msleep(milliseconds)
		LUA_REGISTER(L, "msleep", cc_msleep);

		//
		// string message_encode(msgid, table)
		LUA_REGISTER(L, "message_encode", cc_message_encode);
		//
		// table message_decode(msgid, string)
		LUA_REGISTER(L, "message_decode", cc_message_decode);

		//
		// u32 newtimer(milliseconds, times, ctx, function(id, ctx) end)
		// times: <= 0: forever, > 0: special times
		LUA_REGISTER(L, "newtimer", cc_newtimer);
		//
		// void set_timer_interval(u32, milliseconds)
		LUA_REGISTER(L, "set_timer_interval", cc_set_timer_interval);
		//
		// void set_timer_times(u32, times)
		LUA_REGISTER(L, "set_timer_times", cc_set_timer_times);
		//
		// void remove_timer(u32)
		LUA_REGISTER(L, "remove_timer", cc_remove_timer);

		//
		// void log_XXXX(string)
		LUA_REGISTER(L, "log_debug", cc_syslog_debug);
		LUA_REGISTER(L, "log_trace", cc_syslog_trace);
		LUA_REGISTER(L, "log_alarm", cc_syslog_alarm);
		LUA_REGISTER(L, "log_error", cc_syslog_error);

		//TODO: http
		//TODO: curl
		
		luaT_endNamespace(L);


		//
		// logger
	    luaL_newmetatable(L, LUA_METATABLE_LOGGER_NAME); //logger metatable
	    /* metatable.__index = metatable */
	    lua_pushvalue(L, -1);  /* duplicates the metatable */
	    lua_setfield(L, -2, "__index"); /* __index ref self */
		
		//
		// string log:destination([string dir])
		LUA_REGISTER(L, "destination", cc_log_destination);

		//
		// bool log:toserver(int level, [string address, int port])
		LUA_REGISTER(L, "toserver", cc_log_toserver);
		
		//
		// bool log:tostdout(int level, [bool])
		LUA_REGISTER(L, "tostdout", cc_log_tostdout);
		
		//
		// string log:tofile(int level, [string filename])
		LUA_REGISTER(L, "tofile", cc_log_tofile);

		//
		// bool log:autosplit_day([bool])
		LUA_REGISTER(L, "autosplit_day", cc_log_autosplit_day);

		//
		// bool log:autosplit_hour([bool])
		LUA_REGISTER(L, "autosplit_hour", cc_log_autosplit_hour);
		
		//
		// int log:level([int level])
		//		GLOBAL			=	0,
		//		LEVEL_DEBUG		=	1,
		//		LEVEL_TRACE		=	2,
		//		LEVEL_ALARM		=	3,
		//		LEVEL_ERROR		=	4,
		LUA_REGISTER(L, "level", cc_log_level);

		//
		// void log:XXXXX(string)
		LUA_REGISTER(L, "debug", cc_log_debug);
		LUA_REGISTER(L, "trace", cc_log_trace);
		LUA_REGISTER(L, "alarm", cc_log_alarm);
		LUA_REGISTER(L, "error", cc_log_error);

		LUA_REGISTER(L, "__gc", __cc_log_gc);
		LUA_REGISTER(L, "__tostring", __cc_log_tostring);

		//
		// userdata newlog()
		luaT_beginNamespace(L, LUA_REGISTER_NAMESPACE);
		LUA_REGISTER(L, "newlog", cc_newlog);
		luaT_endNamespace(L);


		//
		// db
	    luaL_newmetatable(L, LUA_METATABLE_DB_NAME); //db metatable
	    /* metatable.__index = metatable */
	    lua_pushvalue(L, -1);  /* duplicates the metatable */
	    lua_setfield(L, -2, "__index"); /* __index ref self */

		//
		// bool db:create_database(database)
		LUA_REGISTER(L, "create_database", cc_db_create_database);
		//
		// bool db:select_database(database)
		LUA_REGISTER(L, "select_database", cc_db_select_database);
		//
		// bool db:loadmsg(filename) // filename also is a directory
		LUA_REGISTER(L, "loadmsg", cc_db_loadmsg);
		//
		// bool db:regtable(table, name) // table => name of protobuf::Message
		LUA_REGISTER(L, "regtable", cc_db_regtable);
		//
		// u64 db:create_object(table, [uint64_t], o)
		LUA_REGISTER(L, "create_object", cc_db_create_object);
		//
		// bool db:delete_object(table, uint64_t)
		LUA_REGISTER(L, "delete_object", cc_db_delete_object);
		//
		// bool db:serialize(table, uint64_t, o)
		LUA_REGISTER(L, "serialize", cc_db_serialize);
		//
		// o db:unserialize(table, uint64_t)
		LUA_REGISTER(L, "unserialize", cc_db_unserialize);
		//
		// bool db:flush_object(table, uint64_t)
		LUA_REGISTER(L, "flush_object", cc_db_flush_object);
		
		LUA_REGISTER(L, "__gc", __cc_db_gc);
		LUA_REGISTER(L, "__tostring", __cc_db_tostring);

		//
		// userdata newdb(host, user, password, port, [database])
		luaT_beginNamespace(L, LUA_REGISTER_NAMESPACE);
		LUA_REGISTER(L, "newdb", cc_newdb);
		luaT_endNamespace(L);

		luaT_cleanup(L);
	}
}

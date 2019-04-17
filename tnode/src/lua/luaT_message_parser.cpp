/*
 * \file: luaT_message_parser.cpp
 * \brief: Created by hushouguo at 17:06:00 Aug 09 2017
 */

#include "tnode.h"
#include "tools/Tools.h"
#include "message/MessageParser.h"
#include "lua/luaT_message_parser.h"

#define DEF_NIL_VALUE		1

BEGIN_NAMESPACE_TNODE {

	bool encodeDescriptor(MessageParser* parser, lua_State* L, Message* message, const Descriptor* descriptor, const Reflection* ref);
	bool decodeDescriptor(MessageParser* parser, lua_State* L, const Message& message, const Descriptor* descriptor, const Reflection* ref);

	//-----------------------------------------------------------------------------------------------------------------------

	bool encodeFieldRepeated(MessageParser* parser, lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref)
	{
		assert(field->is_repeated());
		bool rc = true;
		if (!lua_isnoneornil(L, -1)) {
			switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(CPPTYPE, METHOD_TYPE, VALUE_TYPE, VALUE)	\
				case google::protobuf::FieldDescriptor::CPPTYPE_##CPPTYPE: {\
					   VALUE_TYPE value = VALUE;\
					   ref->Add##METHOD_TYPE(message, field, value);\
				} break;
				CASE_FIELD_TYPE(INT32, Int32, int32_t, lua_tointeger(L, -1));// TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
				CASE_FIELD_TYPE(INT64, Int64, int64_t, lua_tointeger(L, -1));// TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
				CASE_FIELD_TYPE(UINT32, UInt32, uint32_t, lua_tointeger(L, -1));// TYPE_UINT32, TYPE_FIXED32
				CASE_FIELD_TYPE(UINT64, UInt64, uint64_t, lua_tointeger(L, -1));// TYPE_UINT64, TYPE_FIXED64
				CASE_FIELD_TYPE(DOUBLE, Double, double, lua_tonumber(L, -1));// TYPE_DOUBLE
				CASE_FIELD_TYPE(FLOAT, Float, float, lua_tonumber(L, -1));// TYPE_FLOAT
				CASE_FIELD_TYPE(BOOL, Bool, bool, lua_toboolean(L, -1));// TYPE_BOOL
				CASE_FIELD_TYPE(ENUM, EnumValue, int, lua_tointeger(L, -1));// TYPE_ENUM
				CASE_FIELD_TYPE(STRING, String, std::string, lua_tostring(L, -1));// TYPE_STRING, TYPE_BYTES
#undef CASE_FIELD_TYPE

				case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: { // TYPE_MESSAGE, TYPE_GROUP
					Message* submessage = ref->AddMessage(message, field, parser->GetMessageFactory());
					rc = encodeDescriptor(parser, L, submessage, field->message_type(), submessage->GetReflection());
				}
				break;
			}
		}

		return rc;
	}


	bool encodeFieldSimple(MessageParser* parser, lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref) {
		assert(!field->is_repeated());
		bool rc = true;
		if (!lua_isnoneornil(L, -1)) {
			switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(CPPTYPE, METHOD_TYPE, VALUE_TYPE, VALUE)	\
				case google::protobuf::FieldDescriptor::CPPTYPE_##CPPTYPE: {\
					   VALUE_TYPE value = VALUE;\
					   ref->Set##METHOD_TYPE(message, field, value);\
				} break;
				CASE_FIELD_TYPE(INT32, Int32, int32_t, lua_tointeger(L, -1));// TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
				CASE_FIELD_TYPE(INT64, Int64, int64_t, lua_tointeger(L, -1));// TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
				CASE_FIELD_TYPE(UINT32, UInt32, uint32_t, lua_tointeger(L, -1));// TYPE_UINT32, TYPE_FIXED32
				CASE_FIELD_TYPE(UINT64, UInt64, uint64_t, lua_tointeger(L, -1));// TYPE_UINT64, TYPE_FIXED64
				CASE_FIELD_TYPE(DOUBLE, Double, double, lua_tonumber(L, -1));// TYPE_DOUBLE
				CASE_FIELD_TYPE(FLOAT, Float, float, lua_tonumber(L, -1));// TYPE_FLOAT
				CASE_FIELD_TYPE(BOOL, Bool, bool, lua_toboolean(L, -1));// TYPE_BOOL
				CASE_FIELD_TYPE(ENUM, EnumValue, int, lua_tointeger(L, -1));// TYPE_ENUM
				CASE_FIELD_TYPE(STRING, String, std::string, lua_tostring(L, -1));// TYPE_STRING, TYPE_BYTES
#undef CASE_FIELD_TYPE

				case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: { // TYPE_MESSAGE, TYPE_GROUP
					Message* submessage = ref->MutableMessage(message, field, parser->GetMessageFactory());
					rc = encodeDescriptor(parser, L, submessage, field->message_type(), submessage->GetReflection());
				}
				break;
			}
		}

		return rc;
	}

	bool encodeField(MessageParser* parser, lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref) {
		bool rc = true;
		if (field->is_repeated()) {
			lua_pushstring(L, field->name().c_str());/* push key */
			lua_gettable(L, -2);
			if (lua_istable(L, -1)) {
				int table_index = lua_gettop(L);
				lua_pushnil(L);
				while (lua_next(L, table_index) != 0 && rc) {
					/* 'key' is at index -2 and 'value' at index -1, here, `value` is a table */
					//if (!lua_isnumber(L, -2))/* Integer key */
					//{
					//	alarm_log("ignore not-integer key for field:%s\n", field->name().c_str());
					//}
					//ignore `key` type

					rc = encodeFieldRepeated(parser, L, message, field, ref);

					lua_pop(L, 1);/* removes 'value'; keeps 'key' for next iteration */
				}
			}

			lua_pop(L, 1);/* remove `table` or nil */
		}
		else {
			lua_pushstring(L, field->name().c_str());/* key */
			lua_gettable(L, -2);

			rc = encodeFieldSimple(parser, L, message, field, ref);

			lua_pop(L, 1);/* remove `value` or nil */
		}

		return rc;
	}

	bool encodeDescriptor(MessageParser* parser, lua_State* L, Message* message, const Descriptor* descriptor, const Reflection* ref) {
		CHECK_RETURN(lua_istable(L, -1), false, "stack top not table for message: %s", message->GetTypeName().c_str());
		int field_count = descriptor->field_count();
		for (int i = 0; i < field_count; ++i) {
			const FieldDescriptor* field = descriptor->field(i);
			if (!encodeField(parser, L, message, field, ref)) {
				Error("encodeField: %s for message:%s failure", field->name().c_str(), message->GetTypeName().c_str());
				return false;
			}
		}
		return true;
	}


	//-------------------------------------------------------------------------------------------------------

	void decodeFieldDefaultValue(MessageParser* parser, lua_State* L, const Message& message, const FieldDescriptor* field) {
		if (field->is_repeated()) {
			lua_pushstring(L, field->name().c_str());
#ifdef DEF_NIL_VALUE
			lua_pushnil(L);
#else
			lua_newtable(L);
#endif
			lua_settable(L, -3);/* push foo={} */
		}
		else {
			switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(CPPTYPE, METHOD)	\
				case google::protobuf::FieldDescriptor::CPPTYPE_##CPPTYPE: {\
					   lua_pushstring(L, field->name().c_str());\
					   METHOD;\
					   lua_settable(L, -3);\
			    } break;

#ifdef DEF_NIL_VALUE
				CASE_FIELD_TYPE(INT32, lua_pushnil(L));// TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
				CASE_FIELD_TYPE(INT64, lua_pushnil(L));// TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
				CASE_FIELD_TYPE(UINT32, lua_pushnil(L));// TYPE_UINT32, TYPE_FIXED32
				CASE_FIELD_TYPE(UINT64, lua_pushnil(L));// TYPE_UINT64, TYPE_FIXED64
				CASE_FIELD_TYPE(DOUBLE, lua_pushnil(L));// TYPE_DOUBLE
				CASE_FIELD_TYPE(FLOAT, lua_pushnil(L));// TYPE_FLOAT
				CASE_FIELD_TYPE(BOOL, lua_pushnil(L));// TYPE_BOOL
				CASE_FIELD_TYPE(ENUM, lua_pushnil(L));// TYPE_ENUM
				CASE_FIELD_TYPE(STRING, lua_pushnil(L));// TYPE_STRING, TYPE_BYTES
#else
				CASE_FIELD_TYPE(INT32, lua_pushinteger(L, 0));// TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
				CASE_FIELD_TYPE(INT64, lua_pushinteger(L, 0));// TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
				CASE_FIELD_TYPE(UINT32, lua_pushinteger(L, 0));// TYPE_UINT32, TYPE_FIXED32
				CASE_FIELD_TYPE(UINT64, lua_pushinteger(L, 0));// TYPE_UINT64, TYPE_FIXED64
				CASE_FIELD_TYPE(DOUBLE, lua_pushnumber(L, 0.0f));// TYPE_DOUBLE
				CASE_FIELD_TYPE(FLOAT, lua_pushnumber(L, 0.0f));// TYPE_FLOAT
				CASE_FIELD_TYPE(BOOL, lua_pushboolean(L, false));// TYPE_BOOL
				CASE_FIELD_TYPE(ENUM, lua_pushinteger(L, 0));// TYPE_ENUM
				CASE_FIELD_TYPE(STRING, lua_pushstring(L, ""));// TYPE_STRING, TYPE_BYTES
#endif
#undef CASE_FIELD_TYPE

				case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: { // TYPE_MESSAGE, TYPE_GROUP
					lua_pushstring(L, field->name().c_str()); /* key */
#ifdef DEF_NIL_VALUE
					lua_pushnil(L);
#else
					lua_newtable(L);				
#endif
					lua_settable(L, -3);/* push foo={} */
				}
				break;
			}
		}
	}

	bool decodeFieldRepeated(MessageParser* parser, lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref, int index)
	{
		assert(field->is_repeated());
		bool rc = true;
		switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(CPPTYPE, METHOD_TYPE, VALUE_TYPE, METHOD)	\
			case google::protobuf::FieldDescriptor::CPPTYPE_##CPPTYPE: {\
				   lua_pushinteger(L, index);\
				   VALUE_TYPE value = ref->GetRepeated##METHOD_TYPE(message, field, index);\
				   METHOD(L, value);\
				   lua_settable(L, -3);\
		    } break;

			CASE_FIELD_TYPE(INT32, Int32, int32_t, lua_pushinteger);// TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
			CASE_FIELD_TYPE(INT64, Int64, int64_t, lua_pushinteger);// TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
			CASE_FIELD_TYPE(UINT32, UInt32, uint32_t, lua_pushinteger);// TYPE_UINT32, TYPE_FIXED32
			CASE_FIELD_TYPE(UINT64, UInt64, uint64_t, lua_pushinteger);// TYPE_UINT64, TYPE_FIXED64
			CASE_FIELD_TYPE(DOUBLE, Double, double, lua_pushnumber);// TYPE_DOUBLE
			CASE_FIELD_TYPE(FLOAT, Float, float, lua_pushnumber);// TYPE_FLOAT
			CASE_FIELD_TYPE(BOOL, Bool, bool, lua_pushboolean);// TYPE_BOOL
			CASE_FIELD_TYPE(ENUM, EnumValue, int, lua_pushinteger);// TYPE_ENUM
			//CASE_FIELD_TYPE(STRING, String, std::string, lua_pushstring);// TYPE_STRING, TYPE_BYTES
#undef CASE_FIELD_TYPE

			case google::protobuf::FieldDescriptor::CPPTYPE_STRING: { // TYPE_STRING, TYPE_BYTES
				std::string value = ref->GetRepeatedString(message, field, index);
				lua_pushinteger(L, index);
				lua_pushstring(L, value.c_str());
				lua_settable(L, -3);
			}
			break;

			case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: { // TYPE_MESSAGE, TYPE_GROUP
				lua_pushinteger(L, index);
				lua_newtable(L);

				const Message& submessage = ref->GetRepeatedMessage(message, field, index);
				rc = decodeDescriptor(parser, L, submessage, field->message_type(), submessage.GetReflection());

				lua_settable(L, -3);
			}
			break;
		}

		return rc;
	}

	bool decodeFieldSimple(MessageParser* parser, lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref)
	{
		assert(!field->is_repeated());
		bool rc = true;
		switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(CPPTYPE, METHOD_TYPE, VALUE_TYPE, METHOD)	\
			case google::protobuf::FieldDescriptor::CPPTYPE_##CPPTYPE: {\
				   lua_pushstring(L, field->name().c_str());\
				   if (ref->HasField(message, field)) {\
				   	   VALUE_TYPE value = ref->Get##METHOD_TYPE(message, field);\
				   	   METHOD(L, value);\
		   		   } else {\
					   decodeFieldDefaultValue(parser, L, message, field);\
				   }\
				   lua_settable(L, -3);\
		    } break;

			CASE_FIELD_TYPE(INT32, Int32, int32_t, lua_pushinteger);// TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
			CASE_FIELD_TYPE(INT64, Int64, int64_t, lua_pushinteger);// TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
			CASE_FIELD_TYPE(UINT32, UInt32, uint32_t, lua_pushinteger);// TYPE_UINT32, TYPE_FIXED32
			CASE_FIELD_TYPE(UINT64, UInt64, uint64_t, lua_pushinteger);// TYPE_UINT64, TYPE_FIXED64
			CASE_FIELD_TYPE(DOUBLE, Double, double, lua_pushnumber);// TYPE_DOUBLE
			CASE_FIELD_TYPE(FLOAT, Float, float, lua_pushnumber);// TYPE_FLOAT
			CASE_FIELD_TYPE(BOOL, Bool, bool, lua_pushboolean);// TYPE_BOOL
			CASE_FIELD_TYPE(ENUM, EnumValue, int, lua_pushinteger);// TYPE_ENUM
			//CASE_FIELD_TYPE(STRING, String, std::string, lua_pushstring);// TYPE_STRING, TYPE_BYTES
#undef CASE_FIELD_TYPE

			case google::protobuf::FieldDescriptor::CPPTYPE_STRING: { // TYPE_STRING, TYPE_BYTES
				lua_pushstring(L, field->name().c_str());
				std::string value = ref->GetString(message, field);
				lua_pushstring(L, value.c_str());
				lua_settable(L, -3);
			}
			break;

			case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: { // TYPE_MESSAGE, TYPE_GROUP
				lua_pushstring(L, field->name().c_str()); /* key */
				lua_newtable(L);

				const Message& submessage = ref->GetMessage(message, field, parser->GetMessageFactory());
				rc = decodeDescriptor(parser, L, submessage, field->message_type(), submessage.GetReflection());

				lua_settable(L, -3);
			}
			break;
		}

		return rc;
	}

	bool decodeField(MessageParser* parser, lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref) {
		bool rc = true;
		if (field->is_repeated()) {
			lua_pushstring(L, field->name().c_str());
			lua_newtable(L);

			//Debug("message:%s, field:%s, fieldsize:%d", message.GetTypeName().c_str(), field->name().c_str(), ref->FieldSize(message, field));

			for (int i = 0; rc && i < ref->FieldSize(message, field); ++i) {
				rc = decodeFieldRepeated(parser, L, message, field, ref, i);
			}

			lua_settable(L, -3);
		}
		else {
			rc = decodeFieldSimple(parser, L, message, field, ref);
		}

		return rc;
	}

	bool decodeDescriptor(MessageParser* parser, lua_State* L, const Message& message, const Descriptor* descriptor, const Reflection* ref)
	{
		int field_count = descriptor->field_count();
		for (int i = 0; i < field_count; ++i) {
			const FieldDescriptor* field = descriptor->field(i);

			if (!field->is_repeated() && !ref->HasField(message, field)) {
				decodeFieldDefaultValue(parser, L, message, field);
				continue;
			}/* fill default value to lua when a non-repeated field not set, for message field */

			if (!decodeField(parser, L, message, field, ref)) {
				Error("decodeField: %s for message:%s failure", field->name().c_str(), message.GetTypeName().c_str());
				return false;
			}
		}
		return true;
	}


	//
	//------------------------------------------------------------------------------------------------------
	//

	
	//
	// encode lua's table to buffer
	bool luaT_message_parser_encode(MessageParser* parser, lua_State* L, u32 msgid, std::string& out) {
		Message* message = parser->GetMessage(msgid);
		CHECK_RETURN(message, false, "not register message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = parser->FindMessageDescriptor(message);
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());

		assert(message->ByteSize() == 0);
		try {
			if (!encodeDescriptor(parser, L, message, descriptor, message->GetReflection())) {
				Error << "encodeDescriptor failure for message: " << message->GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "encodeDescriptor exception:%s", e.what());
		}

		size_t byteSize = message->ByteSize();

		if (!message->SerializeToString(&out)) {
			Error("Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
			return false;
		}

		return true;	
	}

	bool luaT_message_parser_encode(MessageParser* parser, lua_State* L, u32 msgid, void* buf, size_t& bufsize) {
		Message* message = parser->GetMessage(msgid);
		CHECK_RETURN(message, false, "not register message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = parser->FindMessageDescriptor(message);
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());

		assert(message->ByteSize() == 0);
		try {
			if (!encodeDescriptor(parser, L, message, descriptor, message->GetReflection())) {
				Error << "encodeDescriptor failure for message: " << message->GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "encodeDescriptor exception:%s", e.what());
		}

		size_t byteSize = message->ByteSize();
		CHECK_RETURN(byteSize <= bufsize, false, "bufsize: %ld(need: %ld) overflow for message: %s", bufsize, byteSize, message->GetTypeName().c_str());

		if (!message->SerializeToArray(buf, byteSize)) {
			Error("Serialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), byteSize);
			return false;
		}

		bufsize = byteSize;
		return true;
	}

	Message* luaT_message_parser_encode(MessageParser* parser, lua_State* L, u32 msgid, bool alloc_new) {
		Message* message = nullptr;
		if (alloc_new) {
			message = parser->NewMessage(msgid);
		}
		else {
			message = parser->GetMessage(msgid);
			message->Clear();
		}
		CHECK_RETURN(message, nullptr, "not register message: %d", msgid);
				
		const Descriptor* descriptor = parser->FindMessageDescriptor(message);
		CHECK_RETURN(descriptor, nullptr, "not found descriptor for message: %s", message->GetTypeName().c_str());

		assert(message->ByteSize() == 0);
		try {
			if (!encodeDescriptor(parser, L, message, descriptor, message->GetReflection())) {
				Error << "encodeDescriptor failure for message: " << message->GetTypeName();
				if (alloc_new) {
					SafeDelete(message);
				}
				return nullptr;
			}
		}
		catch(std::exception& e) {
			if (alloc_new) {
				SafeDelete(message);
			}
			CHECK_RETURN(false, nullptr, "encodeDescriptor message exception: %s", e.what());
		}

		return message; 
	}

	//
	//--------------------------------------------------------------------------------------------------
	//
		
	//
	// decode buffer to lua's table
	bool luaT_message_parser_decode(MessageParser* parser, lua_State* L, u32 msgid, const std::string& in) {
		Message* message = parser->GetMessage(msgid);
		CHECK_RETURN(message, false, "Not found message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = parser->FindMessageDescriptor(message);
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());

		bool rc = message->ParseFromString(in);
		CHECK_RETURN(rc, false, "ParseFromString message:%s failure, strlen:%ld", message->GetTypeName().c_str(), in.length());

		lua_newtable(L);
		try {
			if (!decodeDescriptor(parser, L, *message, descriptor, message->GetReflection())) {
				Error << "decodeDescriptor failure for message: " << message->GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "decodeDescriptor exception:%s", e.what());
		}
		return true;
	}

	// decode buffer to lua's table
	bool luaT_message_parser_decode(MessageParser* parser, lua_State* L, u32 msgid, const void* buf, size_t bufsize) {
		Message* message = parser->GetMessage(msgid);
		CHECK_RETURN(message, false, "Not found message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = parser->FindMessageDescriptor(message);
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());

		bool rc = message->ParseFromArray(buf, bufsize);
		CHECK_RETURN(rc, false, "ParseFromArray message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), bufsize);

		lua_newtable(L);
		try {
			if (!decodeDescriptor(parser, L, *message, descriptor, message->GetReflection())) {
				Error << "decodeDescriptor failure for message: " << message->GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "decodeDescriptor exception:%s", e.what());
		}
		return true;
	}

	// decode protobuf::Message to lua's table
	bool luaT_message_parser_decode(MessageParser* parser, lua_State* L, Message* message) {
		const Descriptor* descriptor = parser->FindMessageDescriptor(message);
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());
		lua_newtable(L);
		try {
			if (!decodeDescriptor(parser, L, *message, descriptor, message->GetReflection())) {
				Error << "decodeDescriptor failure for message: " << message->GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "decodeDescriptor exception:%s", e.what());
		}
		return true;
	}
}


/*
 * \file: luaT_message_parser.cpp
 * \brief: Created by hushouguo at 17:06:00 Aug 09 2017
 */

#include "tnode.h"
#include "tools/Tools.h"
#include "lua/luaT_message_parser.h"

#define DEF_NIL_VALUE		1

using namespace google::protobuf;
using namespace google::protobuf::compiler;

BEGIN_NAMESPACE_TNODE {	
	class luaT_message_parser_internal : public luaT_message_parser {
		public:
			luaT_message_parser_internal();
			~luaT_message_parser_internal();

		public:
			bool loadmsg(const char* filename) override;/* filename also is directory */
			bool regmsg(u32 msgid, const char* name) override;

		public:
			bool encode(lua_State* L, u32 msgid, void* buf, size_t& bufsize) override;
			bool encode(lua_State* L, u32 msgid, std::string& out) override;
			bool decode(lua_State* L, u32 msgid, const void* buf, size_t bufsize) override;
			bool decode(lua_State* L, u32 msgid, const std::string& in) override;
			google::protobuf::Message* encode(lua_State* L, u32 msgid) override;

		private:
			DiskSourceTree _tree;
			Importer* _in;
			DynamicMessageFactory _factory;			
			std::unordered_map<u32, Message*> _messages;

			class ImporterErrorCollector : public MultiFileErrorCollector {
				public:
					// implements ErrorCollector ---------------------------------------
					void AddError(const std::string& filename, int line, int column,	const std::string& message) override {
						Error("file: %s:%d:%d, error: %s", filename.c_str(), line, column, message.c_str());
					}

					void AddWarning(const std::string& filename, int line, int column, const std::string& message) override {
						Error("file: %s:%d:%d, error: %s", filename.c_str(), line, column, message.c_str());
					}
			};
			ImporterErrorCollector _errorCollector;

			bool parseProtoFile(const char* filename);
			Message* NewMessage(u32 msgid, const char* name);
			Message* FindMessage(u32 msgid);

		private:
			bool encodeDescriptor(lua_State* L, Message* message, const Descriptor* descriptor, const Reflection* ref);
			bool encodeField(lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref);
			bool encodeFieldSimple(lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref);
			bool encodeFieldRepeated(lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref);

			bool decodeDescriptor(lua_State* L, const Message& message, const Descriptor* descriptor, const Reflection* ref);
			bool decodeField(lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref);
			bool decodeFieldSimple(lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref);
			bool decodeFieldRepeated(lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref, int index);
			void decodeFieldDefaultValue(lua_State* L, const Message& message, const FieldDescriptor* field);
	};

	bool luaT_message_parser_internal::loadmsg(const char* filename) {
		std::function<bool(const char*)> func = [this](const char* fullname)->bool {
			return this->parseProtoFile(fullname);
		};
		return traverseDirectory(filename, ".proto", std::ref(func));
	}

	bool luaT_message_parser_internal::parseProtoFile(const char* filename) {
		const FileDescriptor* fileDescriptor = this->_in->Import(filename);
		CHECK_RETURN(fileDescriptor, false, "import file: %s failure", filename);
		//Debug << "import file: " << filename;
		return true;
	}

	bool luaT_message_parser_internal::regmsg(u32 msgid, const char* name) {
		Message* message = FindOrNull(this->_messages, msgid);
		CHECK_RETURN(message == nullptr, false, "duplicate regmsg: %d, name: %s", msgid, name);
		message = this->NewMessage(msgid, name);
		return message != nullptr;
	}
	
	Message* luaT_message_parser_internal::FindMessage(u32 msgid) {
		return FindOrNull(this->_messages, msgid);
	}
	
	Message* luaT_message_parser_internal::NewMessage(u32 msgid, const char* name) {
		Message* message = this->FindMessage(msgid);
		if (message) {
			return message;
		}		

		const Descriptor* descriptor = this->_in->pool()->FindMessageTypeByName(name);
		CHECK_RETURN(descriptor, nullptr, "not found descriptor for message: %s", name);

		const Message* prototype = this->_factory.GetPrototype(descriptor);
		CHECK_RETURN(prototype, nullptr, "not found prototype for message");

		message = prototype->New();
		this->_messages.insert(std::make_pair(msgid, message));

		//Debug << "message: " << message->DebugString();

		return message;
	}

	//-----------------------------------------------------------------------------------------------------------------------

	bool luaT_message_parser_internal::encodeFieldRepeated(lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref)
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
					Message* submessage = ref->AddMessage(message, field, &this->_factory);
					rc = this->encodeDescriptor(L, submessage, field->message_type(), submessage->GetReflection());
				}
				break;
			}
		}

		return rc;
	}


	bool luaT_message_parser_internal::encodeFieldSimple(lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref) {
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
					Message* submessage = ref->MutableMessage(message, field, &this->_factory);
					rc = this->encodeDescriptor(L, submessage, field->message_type(), submessage->GetReflection());
				}
				break;
			}
		}

		return rc;
	}

	bool luaT_message_parser_internal::encodeField(lua_State* L, Message* message, const FieldDescriptor* field, const Reflection* ref) {
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

					rc = this->encodeFieldRepeated(L, message, field, ref);

					lua_pop(L, 1);/* removes 'value'; keeps 'key' for next iteration */
				}
			}

			lua_pop(L, 1);/* remove `table` or nil */
		}
		else {
			lua_pushstring(L, field->name().c_str());/* key */
			lua_gettable(L, -2);

			rc = this->encodeFieldSimple(L, message, field, ref);

			lua_pop(L, 1);/* remove `value` or nil */
		}

		return rc;
	}

	bool luaT_message_parser_internal::encodeDescriptor(lua_State* L, Message* message, const Descriptor* descriptor, const Reflection* ref) {
		CHECK_RETURN(lua_istable(L, -1), false, "stack top not table for message: %s", message->GetTypeName().c_str());
		int field_count = descriptor->field_count();
		for (int i = 0; i < field_count; ++i) {
			const FieldDescriptor* field = descriptor->field(i);
			if (!this->encodeField(L, message, field, ref)) {
				Error("encodeField: %s for message:%s failure", field->name().c_str(), message->GetTypeName().c_str());
				return false;
			}
		}
		return true;
	}

	google::protobuf::Message* luaT_message_parser_internal::encode(lua_State* L, u32 msgid) {
		Message* message = this->FindMessage(msgid);
		CHECK_RETURN(message, nullptr, "Not found message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = this->_in->pool()->FindMessageTypeByName(message->GetTypeName());
		CHECK_RETURN(descriptor, nullptr, "not found descriptor for message: %s", message->GetTypeName().c_str());
		assert(message->ByteSize() == 0);
		try {
			if (!this->encodeDescriptor(L, message, descriptor, message->GetReflection())) {
				Error << "encodeDescriptor failure for message: " << message->GetTypeName();
				return nullptr;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, nullptr, "encodeDescriptor exception:%s", e.what());
		}

		return message;
	}

	bool luaT_message_parser_internal::encode(lua_State* L, u32 msgid, void* buf, size_t& bufsize) {
		Message* message = this->FindMessage(msgid);
		CHECK_RETURN(message, false, "Not found message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = this->_in->pool()->FindMessageTypeByName(message->GetTypeName());
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());
		assert(message->ByteSize() == 0);
		try {
			if (!this->encodeDescriptor(L, message, descriptor, message->GetReflection())) {
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

	bool luaT_message_parser_internal::encode(lua_State* L, u32 msgid, std::string& out) {
		Message* message = this->FindMessage(msgid);
		CHECK_RETURN(message, false, "Not found message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = this->_in->pool()->FindMessageTypeByName(message->GetTypeName());
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());

		assert(message->ByteSize() == 0);

		try {
			if (!this->encodeDescriptor(L, message, descriptor, message->GetReflection())) {
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

	//-------------------------------------------------------------------------------------------------------

	void luaT_message_parser_internal::decodeFieldDefaultValue(lua_State* L, const Message& message, const FieldDescriptor* field) {
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

	bool luaT_message_parser_internal::decodeFieldRepeated(lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref, int index)
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
				rc = this->decodeDescriptor(L, submessage, field->message_type(), submessage.GetReflection());

				lua_settable(L, -3);
			}
			break;
		}

		return rc;
	}

	bool luaT_message_parser_internal::decodeFieldSimple(lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref)
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
					   this->decodeFieldDefaultValue(L, message, field);\
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

				const Message& submessage = ref->GetMessage(message, field, &this->_factory);
				rc = this->decodeDescriptor(L, submessage, field->message_type(), submessage.GetReflection());

				lua_settable(L, -3);
			}
			break;
		}

		return rc;
	}

	bool luaT_message_parser_internal::decodeField(lua_State* L, const Message& message, const FieldDescriptor* field, const Reflection* ref) {
		bool rc = true;
		if (field->is_repeated()) {
			lua_pushstring(L, field->name().c_str());
			lua_newtable(L);

			//Debug("message:%s, field:%s, fieldsize:%d", message.GetTypeName().c_str(), field->name().c_str(), ref->FieldSize(message, field));

			for (int i = 0; rc && i < ref->FieldSize(message, field); ++i) {
				rc = this->decodeFieldRepeated(L, message, field, ref, i);
			}

			lua_settable(L, -3);
		}
		else {
			rc = this->decodeFieldSimple(L, message, field, ref);
		}

		return rc;
	}

	bool luaT_message_parser_internal::decodeDescriptor(lua_State* L, const Message& message, const Descriptor* descriptor, const Reflection* ref)
	{
		int field_count = descriptor->field_count();
		for (int i = 0; i < field_count; ++i) {
			const FieldDescriptor* field = descriptor->field(i);

			if (!field->is_repeated() && !ref->HasField(message, field)) {
				this->decodeFieldDefaultValue(L, message, field);
				continue;
			}/* fill default value to lua when a non-repeated field not set, for message field */

			if (!this->decodeField(L, message, field, ref)) {
				Error("decodeField: %s for message:%s failure", field->name().c_str(), message.GetTypeName().c_str());
				return false;
			}
		}
		return true;
	}

	bool luaT_message_parser_internal::decode(lua_State* L, u32 msgid, const void* buf, size_t bufsize) {
		Message* message = this->FindMessage(msgid);
		CHECK_RETURN(message, false, "Not found message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = this->_in->pool()->FindMessageTypeByName(message->GetTypeName());
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());

		assert(message->ByteSize() == 0);

		if (!message->ParseFromArray(buf, bufsize)) {
			Error("Unserialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), bufsize);
			return false;
		}

		lua_newtable(L);
		try {
			if (!this->decodeDescriptor(L, *message, descriptor, message->GetReflection())) {
				Error << "decodeDescriptor failure for message: " << message->GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "decodeDescriptor exception:%s", e.what());
		}

		return true;
	}

	bool luaT_message_parser_internal::decode(lua_State* L, u32 msgid, const std::string& in) {
		Message* message = this->FindMessage(msgid);
		CHECK_RETURN(message, false, "Not found message: %d", msgid);
		message->Clear();

		const Descriptor* descriptor = this->_in->pool()->FindMessageTypeByName(message->GetTypeName());
		CHECK_RETURN(descriptor, false, "not found descriptor for message: %s", message->GetTypeName().c_str());

		assert(message->ByteSize() == 0);

		if (!message->ParseFromString(in)) {
			Error("Unserialize message:%s failure, byteSize:%ld", message->GetTypeName().c_str(), in.length());
			return false;
		}

		lua_newtable(L);
		try {
			if (!this->decodeDescriptor(L, *message, descriptor, message->GetReflection())) {
				Error << "decodeDescriptor failure for message: " << message->GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "decodeDescriptor exception:%s", e.what());
		}

		return true;
	}


	luaT_message_parser::~luaT_message_parser() {}
	luaT_message_parser_internal::luaT_message_parser_internal() {
		this->_tree.MapPath("", "./");
		this->_in = new Importer(&this->_tree, &this->_errorCollector);
	}
	luaT_message_parser_internal::~luaT_message_parser_internal() {
		for (auto& i : this->_messages) {
			delete i.second;
		}
		this->_messages.clear();
		SafeDelete(this->_in);
	}
	
	luaT_message_parser* luaT_message_parser_creator::create() {
		return new luaT_message_parser_internal();
	}	
}


/*
 * \file: EasydbParser.cpp
 * \brief: Created by hushouguo at 14:34:00 Apr 15 2019
 */

#include "tnode.h"

BEGIN_NAMESPACE_TNODE {
	static void easydb_parser_decode_field_simple_default_value(const Message& message, const FieldDescriptor* field, Object& object) {
		if (field->is_repeated()) {
			object.setValue(field->name().c_str(), "[]");
		}
		else {
			switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(CPPTYPE, VALUE)	\
				case FieldDescriptor::CPPTYPE_##CPPTYPE: {\
						object.setValue(field->name().c_str(), VALUE);\
				} break;
	
				CASE_FIELD_TYPE(INT32, int32_t(0));// TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
				CASE_FIELD_TYPE(INT64, int64_t(0));// TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
				CASE_FIELD_TYPE(UINT32, uint32_t(0));// TYPE_UINT32, TYPE_FIXED32
				CASE_FIELD_TYPE(UINT64, uint64_t(0));// TYPE_UINT64, TYPE_FIXED64
				CASE_FIELD_TYPE(DOUBLE, double(0.0f));// TYPE_DOUBLE
				CASE_FIELD_TYPE(FLOAT, float(0.0f));// TYPE_FLOAT
				CASE_FIELD_TYPE(BOOL, false);// TYPE_BOOL
				CASE_FIELD_TYPE(ENUM, int(0));// TYPE_ENUM
				CASE_FIELD_TYPE(STRING, "");// TYPE_STRING, TYPE_BYTES
				
#undef CASE_FIELD_TYPE
	
				case FieldDescriptor::CPPTYPE_MESSAGE: { // TYPE_MESSAGE, TYPE_GROUP
					object.setValue(field->name().c_str(), "{}");
				}
				break;
			}
		}
	}
	
	static bool easydb_parser_decode_field_simple_repeated(const Message& message, const FieldDescriptor* field, const Reflection* ref, int index, Object& object) {
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
	
	static bool easydb_parser_decode_field_simple(const Message& message, const Descriptor* descriptor, const Reflection* ref, Object& object) {
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
	
	static bool easydb_parser_decode_field(const Message& message, const Descriptor* descriptor, const Reflection* ref, Object& object) {
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
			rc = easydb_parser_decode_field_simple(L, message, field, ref);
		}
	
		return rc;
	}	

	bool easydb_parser_decode(const Message& message, const Descriptor* descriptor, const Reflection* ref, Object& object) {
		object.clear();		
		int field_count = descriptor->field_count();
		for (int i = 0; i < field_count; ++i) {
			const FieldDescriptor* field = descriptor->field(i);		
			if (!field->is_repeated() && !ref->HasField(message, field)) {
				this->decodeFieldDefaultValue(L, message, field);
				continue;
			}/* fill default value to lua when a non-repeated field not set, for message field */
		
			if (!easydb_parser_decode_field(message, field, ref, object)) {
				Error("decodeField: %s for message:%s failure", field->name().c_str(), message.GetTypeName().c_str());
				return false;
			}
		}
		return true;
	}
}


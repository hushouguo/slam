/*
 * \file: EasydbParser.h
 * \brief: Created by hushouguo at 14:33:58 Apr 15 2019
 */
 
#ifndef __EASYDBPARSER_H__
#define __EASYDBPARSER_H__

enum CppType {
  CPPTYPE_INT32 	  = 1,	   // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
  CPPTYPE_INT64 	  = 2,	   // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
  CPPTYPE_UINT32	  = 3,	   // TYPE_UINT32, TYPE_FIXED32
  CPPTYPE_UINT64	  = 4,	   // TYPE_UINT64, TYPE_FIXED64
  CPPTYPE_DOUBLE	  = 5,	   // TYPE_DOUBLE
  CPPTYPE_FLOAT 	  = 6,	   // TYPE_FLOAT
  CPPTYPE_BOOL		  = 7,	   // TYPE_BOOL
  CPPTYPE_ENUM		  = 8,	   // TYPE_ENUM
  CPPTYPE_STRING	  = 9,	   // TYPE_STRING, TYPE_BYTES
  CPPTYPE_MESSAGE	  = 10,    // TYPE_MESSAGE, TYPE_GROUP

  MAX_CPPTYPE		  = 10,    // Constant useful for defining lookup tables
							   // indexed by CppType.
};

BEGIN_NAMESPACE_TNODE {
	struct Object {
		u64 id;
		struct Value {
			FieldDescriptor::CppType type;
			union {
				int32_t		value_s32;		// CPPTYPE_INT32: TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
				int64_t 	value_s64;		// CPPTYPE_INT64: TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
				uint32_t 	value_u32;		// CPPTYPE_UINT32: TYPE_UINT32, TYPE_FIXED32
				uint64_t 	value_u64;		// CPPTYPE_UINT64: TYPE_UINT64, TYPE_FIXED64
				double 		value_double;	// CPPTYPE_DOUBLE
				float 		value_float;	// CPPTYPE_FLOAT
				bool 		value_bool;		// CPPTYPE_BOOL
				int 		value_enum;		// CPPTYPE_ENUM
			};
			//
			// repeated, message encode to json
			ByteBuffer 		value_string;	// CPPTYPE_STRING: TYPE_STRING, TYPE_BYTES, TYPE_MESSAGE, TYPE_GROUP
			bool dirty;
		};
		std::unordered_map<std::string, Value> values;

		inline void clear() {
			this->id = 0;
			this->values.clear();
		}

		template <typename T> void setValue(const char* name, T value) {
			Error << "Object::setValue template specialization failure for type: " << typeid(T).name();
			assert(false);
		}

		// boolean
		template <> void setValue(const char* name, bool value);

		// integer
		template <> void setValue(const char* name, s8 value);
		template <> void setValue(const char* name, s16 value);
		template <> void setValue(const char* name, s32 value);
		template <> void setValue(const char* name, s64 value);
		template <> void setValue(const char* name, u8 value);
		template <> void setValue(const char* name, u16 value);
		template <> void setValue(const char* name, u32 value);
		template <> void setValue(const char* name, u64 value);
		template <> void setValue(const char* name, long long value);
		template <> void setValue(const char* name, unsigned long long value);

		// float
		template <> void setValue(const char* name, float value);
		template <> void setValue(const char* name, double value);

		// string
		template <> void setValue(const char* name, char* value);
		template <> void setValue(const char* name, const char* value);
		template <> void setValue(const char* name, std::string value);

		// bytes
		template <> void setValue(const char* name, void* value);
		template <> void setValue(const char* name, const void* value);		
	};
	
	bool easydb_parser_decode(const Message& message, const Descriptor* descriptor, const Reflection* ref, Object& object);
}

#endif

/*
 * \file: Easydb.h
 * \brief: Created by hushouguo at 09:34:27 Apr 12 2019
 */
 
#ifndef __EASYDB_H__
#define __EASYDB_H__

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include <exception>
#include <fstream>  
#include <sstream>

#define EASYDB_VERSION_MAJOR		0
#define EASYDB_VERSION_MINOR		1
#define EASYDB_VERSION_PATCH		0

#if defined(__plusplus)
#if __cplusplus <= 199711L
#error "REQUIRE C++ 11 SUPPORT"
#endif
#endif

namespace db {
	struct Entity {
		uint64_t id;
		struct Value {
			enum Type {
				type_null			=	0,	// null
				type_integer		=	1,	// 4, 8 bytes
				type_float			=	2,	// 4 bytes float
				type_bool			=	3,	// 1 byte boolean
				type_string			=	4	// string
			};
			Type type;
			int64_t value_integer;
			float value_float;
			bool value_bool;
			std::string value_string;
			Value() : type(type_null) {}
			Value(int8_t value) { this->Set(value); }
			Value(uint8_t value) { this->Set(value); }
			Value(int16_t value) { this->Set(value); }
			Value(uint16_t value) { this->Set(value); }
			Value(int32_t value) { this->Set(value); }
			Value(uint32_t value) { this->Set(value); }
			Value(int64_t value) { this->Set(value); }
			Value(uint64_t value) { this->Set(value); }
			Value(float value) { this->Set(value); }
			Value(double value) { this->Set(value); }
			Value(bool value) { this->Set(value); }
			Value(char* value) { this->Set(value); }
			Value(const char* value) { this->Set(value); }
			Value(std::string value) { this->Set(value); }
			Value(const std::string& value) { this->Set(value); }
			inline void Set(int8_t value) { this->type = type_integer; this->value_integer = value; dirty = true; }
			inline void Set(uint8_t value) { this->type = type_integer; this->value_integer = value; dirty = true; }
			inline void Set(int16_t value) { this->type = type_integer; this->value_integer = value; dirty = true; }
			inline void Set(uint16_t value) { this->type = type_integer; this->value_integer = value; dirty = true; }
			inline void Set(int32_t value) { this->type = type_integer; this->value_integer = value; dirty = true; }
			inline void Set(uint32_t value) { this->type = type_integer; this->value_integer = value; dirty = true; }
			inline void Set(int64_t value) { this->type = type_integer; this->value_integer = value; dirty = true; }
			inline void Set(uint64_t value) { this->type = type_integer; this->value_integer = value; dirty = true; }
			inline void Set(float value) { this->type = type_float; this->value_float = value; dirty = true; }
			inline void Set(double value) { this->type = type_float; this->value_float = (float)value; dirty = true; }
			inline void Set(bool value) { this->type = type_bool; this->value_bool = value; dirty = true; }
			inline void Set(char* value) { this->type = type_string; this->value_string = value; dirty = true; }
			inline void Set(const char* value) { this->type = type_string; this->value_string = value; dirty = true; }
			inline void Set(std::string value) { this->type = type_string; this->value_string = value; dirty = true; }
			inline void Set(const std::string& value) { this->type = type_string; this->value_string = value; dirty = true; }
			template <typename T> void operator = (T value) { this->Set(value); }
		};
		bool dirty = false;
		std::unordered_map<std::string, Value> values;

		int64_t GetInteger(const char*);
		bool GetBool(const char*);
		float GetFloat(const char*);
		const std::string& GetString(const char*);

		Value& GetValue(const char*);
		Value& operator [](const char*);

		bool HasMember(const char*);

		bool IsNull(const char*);
		bool IsNumber(const char*);	// IsInteger() || IsFloat()
		bool IsInteger(const char*);
		bool IsFloat(const char*);
		bool IsBool(const char*);
		bool IsString(const char*);

		void Dump();
		void Clear();
		void ClearDirty();
	};

	class Easydb {
		public:
			virtual ~Easydb() = 0;

		public:
			virtual int localServer(const char* host, const char* user, const char* passwd, int port) = 0;
			virtual int connectServer(const char* address, int port) = 0;

		public:
			virtual bool selectDatabase(std::string) = 0;

		public:
			virtual bool addEntity(int handle, std::string table, const Entity* entity) = 0;
			virtual bool modifyEntity(int handle, std::string table, const Entity* entity) = 0;
			virtual bool loadEntity(int handle, std::string table, uint64_t entityid, Entity* entity) = 0;
			virtual bool removeEntity(int handle, uint64_t entityid) = 0;
			virtual bool runQuery(int handle, std::string where, std::vector<Entity*>& entities) = 0;

		public:
			// MUL KEY
			virtual bool addKey(int handle, std::string table, std::string field) = 0;
			virtual bool removeKey(int handle, std::string table, std::string field) = 0;
			// UNI KEY
			virtual bool addUnique(int handle, std::string table, std::string field) = 0;
			virtual bool removeUnique(int handle, std::string table, std::string field) = 0;
			// UNSIGNED
			virtual bool addUnsigned(int handle, std::string table, std::string field) = 0;
			virtual bool removeUnsigned(int handle, std::string table, std::string field) = 0;
	};
}

#endif

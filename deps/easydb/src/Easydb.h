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

#define EASYDB_ENABLE_DEBUG			1

#if defined(__plusplus)
#if __cplusplus <= 199711L
#error "REQUIRE C++ 11 SUPPORT"
#endif
#endif

namespace db {
	struct Entity {
		uint64_t id;
		Entity(uint64_t entityid);
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
			Value();
			Value(int8_t value);
			Value(uint8_t value);
			Value(int16_t value);
			Value(uint16_t value);
			Value(int32_t value);
			Value(uint32_t value);
			Value(int64_t value);
			Value(uint64_t value);
			Value(float value);
			Value(double value);
			Value(bool value);
			Value(char* value);
			Value(const char* value);
			Value(std::string value);
			Value(const std::string& value);
			void Set(int8_t value);
			void Set(uint8_t value);
			void Set(int16_t value);
			void Set(uint16_t value);
			void Set(int32_t value);
			void Set(uint32_t value);
			void Set(int64_t value);
			void Set(uint64_t value);
			void Set(float value);
			void Set(double value);
			void Set(bool value);
			void Set(char* value);
			void Set(const char* value);
			void Set(std::string value);
			void Set(const std::string& value);
			template <typename T> void operator = (T value) { this->Set(value); }
			bool dirty = false;
		};
		std::unordered_map<std::string, Value> values;

		int64_t GetInteger(const char* key);
		bool GetBool(const char* key);
		float GetFloat(const char* key);
		const std::string& GetString(const char* key);

		Value& GetValue(const char* key);
		Value& operator [](const char* key);

		bool HasMember(const char* key);

		bool IsNull(const char* key);
		bool IsNumber(const char* key);	// IsInteger() || IsFloat()
		bool IsInteger(const char* key);
		bool IsFloat(const char* key);
		bool IsBool(const char* key);
		bool IsString(const char* key);

		void Dump();
		void Clear();
		void ClearDirty();
	};

	class Easydb {
		public:
			virtual ~Easydb() = 0;

		public:
			virtual bool connectServer(const char* host, const char* user, const char* passwd, int port) = 0;
			virtual bool connectServer(const char* address, int port) = 0;

		public:
			virtual bool createDatabase(std::string) = 0;
			virtual bool selectDatabase(std::string) = 0;

		public:
			virtual bool serialize(std::string table, const Entity* entity) = 0;
			virtual Entity* unserialize(std::string table, uint64_t entityid) = 0;
			virtual bool removeEntity(std::string table, uint64_t entityid) = 0;
			virtual bool runQuery(std::string where, std::vector<Entity*>& entities) = 0;

		public:
			// MUL KEY
			virtual bool addKey(std::string table, std::string field) = 0;
			virtual bool removeKey(std::string table, std::string field) = 0;
			// UNI KEY
			virtual bool addUnique(std::string table, std::string field) = 0;
			virtual bool removeUnique(std::string table, std::string field) = 0;
			// UNSIGNED
			virtual bool addUnsigned(std::string table, std::string field) = 0;
			virtual bool removeUnsigned(std::string table, std::string field) = 0;

		public:
			static Easydb* createInstance();
	};
}

#endif

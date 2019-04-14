/*
 * \file: Easydb.h
 * \brief: Created by hushouguo at 09:34:27 Apr 12 2019
 */
 
#ifndef __EASYDB_H__
#define __EASYDB_H__

#include <stdint.h>
#include <string>
#include <vector>
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
		Entity();
		Entity(uint64_t entityid);
		struct Value {
			enum Type {
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
			bool dirty = false;
			inline size_t Size() {
				switch (this->type) {
					case type_integer: return sizeof(this->value_integer);
					case type_float: return sizeof(this->value_float);
					case type_bool: return 1;// bool fixed 1 byte
					case type_string: return this->value_string.length();
					default: assert(false);
				}				
			}
			inline void* Buffer() {
				switch (this->type) {
					case type_integer: return &this->value_integer;
					case type_float: return &this->value_float;
					case type_bool: return &this->value_bool;
					case type_string: return this->value_string.data();
					default: assert(false);
				}				
			}
		};

		//
		// SetValue
		template <typename T> void setValue(const char* key, T value) {
			Error("setValue template specialization failure for type: %s", typeid(T).name());
			assert(false);			
		}
		
		// boolean
		template <> void setValue(const char* key, bool value);
		
		// integer
		template <> void setValue(const char* key, s8 value);
		template <> void setValue(const char* key, s16 value);
		template <> void setValue(const char* key, s32 value);
		template <> void setValue(const char* key, s64 value);
		template <> void setValue(const char* key, u8 value);
		template <> void setValue(const char* key, u16 value);
		template <> void setValue(const char* key, u32 value);
		template <> void setValue(const char* key, u64 value);
		template <> void setValue(const char* key, long long value);
		template <> void setValue(const char* key, unsigned long long value);
		
		// float
		template <> void setValue(const char* key, float value);
		template <> void setValue(const char* key, double value);
		
		// string
		template <> void setValue(const char* key, char* value);
		template <> void setValue(const char* key, const char* value);
		template <> void setValue(const char* key, std::string value);
		

		//
		// getValue
		int64_t getValueInteger(const char* key);
		bool getValueBool(const char* key);
		float getValueFloat(const char* key);
		const std::string& getValueString(const char* key);

		//
		// check key if exists
		bool hasMember(const char* key);
		bool isNull(const char* key);
		bool isNumber(const char* key);	// IsInteger() || IsFloat()
		bool isInteger(const char* key);
		bool isFloat(const char* key);
		bool isBool(const char* key);
		bool isString(const char* key);

		size_t valueSize();
		size_t dirtyValueSize();
		void dump();
		void clear();
		void clearDirty();

		inline uint64_t ID() { return this->_id; }
		inline void ID(uint64_t entityid) { this->_id = entityid; }
		inline const std::unordered_map<std::string, Value>& values() { return this->_values; }
		
		private:
			uint64_t _id = 0;
			std::unordered_map<std::string, Value> _values;
	};

	struct EasydbConfig {
		std::string host;
		std::string user;
		std::string passwd;
		std::string database;
		int port;
	};
	class Easydb {
		public:
			virtual ~Easydb() = 0;
			virtual void stop() = 0;

		public:
			virtual bool connectServer(EasydbConfig* conf) = 0;

		public:
			virtual bool createDatabase(std::string) = 0;
			virtual bool selectDatabase(std::string) = 0;
			virtual bool deleteDatabase(std::string) = 0;
			virtual bool findDatabase(std::string) = 0;

		public:
			virtual uint64_t createEntity(std::string table, Entity* entity) = 0;
			virtual Entity* getEntity(std::string table, uint64_t entityid) = 0;
			virtual bool deleteEntity(std::string table, uint64_t entityid) = 0;
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

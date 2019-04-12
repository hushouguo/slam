/*
 * \file: Easydb.cpp
 * \brief: Created by hushouguo at 11:25:15 Apr 12 2019
 */

#include "Database.h"

namespace db {
	Entity::Entity(uint64_t entityid) {
		this->id = entityid;
	}
	
	Entity::Value::Value() : type(type_null) {}
	Entity::Value::Value(int8_t value) { this->Set(value); }
	Entity::Value::Value(uint8_t value) { this->Set(value); }
	Entity::Value::Value(int16_t value) { this->Set(value); }
	Entity::Value::Value(uint16_t value) { this->Set(value); }
	Entity::Value::Value(int32_t value) { this->Set(value); }
	Entity::Value::Value(uint32_t value) { this->Set(value); }
	Entity::Value::Value(int64_t value) { this->Set(value); }
	Entity::Value::Value(uint64_t value) { this->Set(value); }
	Entity::Value::Value(float value) { this->Set(value); }
	Entity::Value::Value(double value) { this->Set(value); }
	Entity::Value::Value(bool value) { this->Set(value); }
	Entity::Value::Value(char* value) { this->Set(value); }
	Entity::Value::Value(const char* value) { this->Set(value); }
	Entity::Value::Value(std::string value) { this->Set(value); }
	//Entity::Value::Value(const std::string& value) { this->Set(value); }
	
	void Entity::Value::Set(int8_t value) { type = type_integer; value_integer = value; dirty = true; }
	void Entity::Value::Set(uint8_t value) { type = type_integer; value_integer = value; dirty = true; }
	void Entity::Value::Set(int16_t value) { type = type_integer; value_integer = value; dirty = true; }
	void Entity::Value::Set(uint16_t value) { type = type_integer; value_integer = value; dirty = true; }
	void Entity::Value::Set(int32_t value) { type = type_integer; value_integer = value; dirty = true; }
	void Entity::Value::Set(uint32_t value) { type = type_integer; value_integer = value; dirty = true; }
	void Entity::Value::Set(int64_t value) { type = type_integer; value_integer = value; dirty = true; }
	void Entity::Value::Set(uint64_t value) { type = type_integer; value_integer = value; dirty = true; }
	void Entity::Value::Set(float value) { type = type_float; value_float = value; dirty = true; }
	void Entity::Value::Set(double value) { type = type_float; value_float = (float)value; dirty = true; }
	void Entity::Value::Set(bool value) { type = type_bool; value_bool = value; dirty = true; }
	void Entity::Value::Set(char* value) { type = type_string; value_string = value; dirty = true; }
	void Entity::Value::Set(const char* value) { type = type_string; value_string = value; dirty = true; }
	void Entity::Value::Set(std::string value) { type = type_string; value_string = value; dirty = true; }
	//void Entity::Value::Set(const std::string& value) { type = type_string; value_string = value; dirty = true; }

#define GET_VALUE(NAME, TYPE)\
		({\
			auto __i = this->values.find(NAME);\
			assert(__i != this->values.end());\
			const Entity::Value& __value = __i->second;\
			assert(__value.type == Entity::Value::type_##TYPE);\
			return __value.value_##TYPE;\
		})

	int64_t Entity::GetInteger(const char* key) {
		GET_VALUE(key, integer);
	}
	
	bool Entity::GetBool(const char* key) {
		GET_VALUE(key, bool);
	}
	
	float Entity::GetFloat(const char* key) {
		GET_VALUE(key, float);
	}
	
	const std::string& Entity::GetString(const char* key) {
		GET_VALUE(key, string);
	}

	Entity::Value& Entity::GetValue(const char* key) {
		return this->values[key];
	}
	
	Entity::Value& Entity::operator [](const char* key) {
		return this->values[key];
	}

	bool Entity::HasMember(const char* key) {
		return this->values.find(key) != this->values.end();
	}

#define CHECK_VALUE_TYPE(name, value_type)\
		({\
			auto __i = this->values.find(name);\
			bool __rc = __i != this->values.end();\
			if (__rc) {\
				const Entity::Value& __value = __i->second;\
				__rc = __value.type == value_type;\
			}\
			else { \
				assert(false);\
			}\
			__rc;\
		})

	bool Entity::IsNull(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_null);
	}
	
	bool Entity::IsNumber(const char* key) {
		return this->IsInteger(key) || this->IsFloat(key);
	}
	
	bool Entity::IsInteger(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_integer);
	}
	
	bool Entity::IsFloat(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_float);
	}
	
	bool Entity::IsBool(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_bool);
	}
	
	bool Entity::IsString(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_string);
	}

	void Entity::Dump() {
		fprintf(stderr, "Entity: 0x%lx, attributes size: %ld\n", this->id, this->values.size());
		for (auto& i : this->values) {
			const Entity::Value& value = i.second;
			switch (value.type) {
				case Entity::Value::type_null: fprintf(stderr, "%s: null\n", i.first.c_str()); break;
				case Entity::Value::type_integer: 
					fprintf(stderr, "%s: %ld(int)%s\n", i.first.c_str(), value.value_integer, (value.dirty ? "*" : ""));
					break;

				case Entity::Value::type_float:
					fprintf(stderr, "%s: %.2f(float)%s\n", i.first.c_str(), value.value_float, (value.dirty ? "*" : ""));
					break;
				
				case Entity::Value::type_bool:
					fprintf(stderr, "%s: %s(bool)%s\n", i.first.c_str(), (value.value_bool ? "true" : "false"), (value.dirty ? "*" : ""));
					break;
					
				case Entity::Value::type_string:
					fprintf(stderr, "%s: %s(string)%s\n", i.first.c_str(), value.value_string.c_str(), (value.dirty ? "*" : ""));
					break;
					
				default: assert(false); break;
			}
		}
	}
	
	void Entity::Clear() {
		this->values.clear();
	}
	
	void Entity::ClearDirty() {
		for (auto& i : this->values) {
			i.second.dirty = false;
		}		
	}
	
	Easydb* Easydb::createInstance() {
		return new EasydbInternal();
	}
}


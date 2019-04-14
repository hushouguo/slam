/*
 * \file: Easydb.cpp
 * \brief: Created by hushouguo at 11:25:15 Apr 12 2019
 */

#include "Database.h"

namespace db {
	Entity::Entity() {
	}
	
	Entity::Entity(uint64_t entityid) {
		this->_id = entityid;
	}

	//
	// setValue
	//
	
	// boolean
	template <> void Entity::setValue(const char* key, bool value) {
		const Entity::Value& __value = this->_values[key];
		__value.type = Entity::Value::type_bool;
		__value.value_bool = value;
		__value.dirty = true;
	}
	
	// integer
	template <> void Entity::setValue(const char* key, s8 value) {
		this->setValue(static_cast<int64_t>(value));
	}
	
	template <> void Entity::setValue(const char* key, s16 value) {
		this->setValue(static_cast<int64_t>(value));
	}
		
	template <> void Entity::setValue(const char* key, s32 value) {
		this->setValue(static_cast<int64_t>(value));
	}
	
	template <> void Entity::setValue(const char* key, s64 value) {
		const Entity::Value& __value = this->_values[key];
		__value.type = Entity::Value::type_integer;
		__value.value_integer = value;
		__value.dirty = true;
	}
	
	template <> void Entity::setValue(const char* key, u8 value) {
		this->setValue(static_cast<int64_t>(value));
	}
	
	template <> void Entity::setValue(const char* key, u16 value) {
		this->setValue(static_cast<int64_t>(value));
	}
	
	template <> void Entity::setValue(const char* key, u32 value) {
		this->setValue(static_cast<int64_t>(value));
	}
	
	template <> void Entity::setValue(const char* key, u64 value) {
		this->setValue(static_cast<int64_t>(value));
	}
	
	template <> void Entity::setValue(const char* key, long long value) {
		this->setValue(static_cast<int64_t>(value));
	}
	
	template <> void Entity::setValue(const char* key, unsigned long long value) {
		this->setValue(static_cast<int64_t>(value));
	}
	
	
	// float
	template <> void Entity::setValue(const char* key, float value) {
		const Entity::Value& __value = this->_values[key];
		__value.type = Entity::Value::type_float;
		__value.value_float = value;
		__value.dirty = true;
	}
	
	template <> void Entity::setValue(const char* key, double value) {
		this->setValue(static_cast<float>(value));
	}
	
	
	// string
	template <> void Entity::setValue(const char* key, char* value) {
		this->setValue(static_cast<const char*>(value));
	}
	
	template <> void Entity::setValue(const char* key, const char* value) {
		const Entity::Value& __value = this->_values[key];
		__value.type = Entity::Value::type_string;
		__value.value_string = value;
		__value.dirty = true;
	}
	
	template <> void Entity::setValue(const char* key, std::string value) {
		this->setValue(static_cast<const char*>(value));
	}


#define GET_VALUE(NAME, TYPE)\
			({\
				auto __i = this->_values.find(NAME);\
				assert(__i != this->_values.end());\
				const Entity::Value& __value = __i->second;\
				assert(__value.type == Entity::Value::type_##TYPE);\
				return __value.value_##TYPE;\
			})

	//
	// getValue
	//
	int64_t Entity::getValueInteger(const char* key) {		
		GET_VALUE(key, integer);
	}
	
	bool Entity::getValueBool(const char* key) {
		GET_VALUE(key, bool);
	}
	
	float Entity::getValueFloat(const char* key) {
		GET_VALUE(key, float);
	}
	
	const std::string& Entity::getValueString(const char* key) {
		GET_VALUE(key, string);
	}
	

//	Entity::Value& Entity::GetValue(const char* key) {
//		return this->_values[key];
//	}
	
//	Entity::Value& Entity::operator [](const char* key) {
//		return this->_values[key];
//	}

	bool Entity::hasMember(const char* key) {
		return this->_values.find(key) != this->_values.end();
	}

#define CHECK_VALUE_TYPE(name, value_type)\
		({\
			auto __i = this->_values.find(name);\
			bool __rc = __i != this->_values.end();\
			if (__rc) {\
				const Entity::Value& __value = __i->second;\
				__rc = __value.type == value_type;\
			}\
			else { \
				assert(false);\
			}\
			__rc;\
		})

	bool Entity::isNull(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_null);
	}
	
	bool Entity::isNumber(const char* key) {
		return this->isInteger(key) || this->isFloat(key);
	}
	
	bool Entity::isInteger(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_integer);
	}
	
	bool Entity::isFloat(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_float);
	}
	
	bool Entity::isBool(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_bool);
	}
	
	bool Entity::isString(const char* key) {
		return CHECK_VALUE_TYPE(key, Entity::Value::type_string);
	}

	void Entity::dump() {
		fprintf(stderr, "Entity: 0x%lx, attributes size: %ld\n", this->id, this->_values.size());
		for (auto& i : this->_values) {
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
	
	void Entity::clear() {
		this->_values.clear();
	}
	
	void Entity::clearDirty() {
		for (auto& i : this->_values) {
			i.second.dirty = false;
		}		
	}

	size_t Entity::valueSize() {
		return this->_values.size();
	}
	
	size_t Entity::dirtyValueSize() {
		size_t size = 0;
		for (auto& i : this->_values) {
			if (i.second.dirty) {
				++size;
			}
		}
		return size;
	}
	
	Easydb* Easydb::createInstance() {
		return new EasydbInternal();
	}
}


/*
 * \file: Helper.cpp
 * \brief: Created by hushouguo at 15:10:55 Apr 12 2019
 */

#include "Database.h"

#define DEF_VARCHAR_LENGTH	128
#define DEF_TEXT_LENGTH		65535

namespace db {
	//
	// type of field => Entity::Value::Type
	//
	static std::map<enum_field_types, Entity::Value::Type> m2v = {
		{ MYSQL_TYPE_TINY, Entity::Value::type_bool },
		{ MYSQL_TYPE_LONG, Entity::Value::type_integer },
		{ MYSQL_TYPE_LONGLONG, Entity::Value::type_integer },
		{ MYSQL_TYPE_FLOAT, Entity::Value::type_float },
		{ MYSQL_TYPE_VAR_STRING, Entity::Value::type_string },
		{ MYSQL_TYPE_BLOB, Entity::Value::type_string },
		{ MYSQL_TYPE_LONG_BLOB, Entity::Value::type_string },
	};
	Entity::Value::Type convert(enum_field_types type) {
		auto i = m2v.find(type);
		assert(i != m2v.end());
		return m2v[type];
	}
	
	bool valid_type(enum_field_types type) {
		return m2v.find(type) != m2v.end();
	}


	//
	// Entity::Value::Type => type of field
	//
	// TINY			-> tinyint		->	bool
	// LONG			-> int			->	integer
	// LONGLONG		-> bigint		->	integer
	// FLOAT		-> float		->	float
	// VAR_STRING	-> varchar(128)	->	string
	// BLOB			-> text			->	string
	// LONGBLOB		-> longtext		->	string
	//
	static std::map<Entity::Value::Type, std::function<enum_field_types(const Entity::Value&)>> v2m = {
		{ Entity::Value::type_integer, [](const Entity::Value& value) -> enum_field_types {
			assert(value.type == Entity::Value::type_integer);
			return (value.value_integer > INT32_MAX || value.value_integer < INT32_MIN) ? MYSQL_TYPE_LONGLONG : MYSQL_TYPE_LONG;
		}},

		{ Entity::Value::type_bool, [](const Entity::Value& value) -> enum_field_types {
			assert(value.type == Entity::Value::type_bool);
			return MYSQL_TYPE_TINY;
		}},

		{ Entity::Value::type_float, [](const Entity::Value& value) -> enum_field_types {
			assert(value.type == Entity::Value::type_float);
			return MYSQL_TYPE_FLOAT;
		}},	

		{ Entity::Value::type_string, [](const Entity::Value& value) -> enum_field_types {
			assert(value.type == Entity::Value::type_string);
			return value.value_string.length() > DEF_TEXT_LENGTH ? MYSQL_TYPE_LONG_BLOB 
						: (value.value_string.length() > DEF_VARCHAR_LENGTH ? MYSQL_TYPE_BLOB : MYSQL_TYPE_VAR_STRING);
		}}
	};
	enum_field_types convert(const Entity::Value& value) {
		auto i = v2m.find(value.type);
		assert(i != v2m.end());
		return v2m[value.type](value);
	}
	
	bool valid_type(Entity::Value::Type type) {
		return v2m.find(type) != v2m.end();
	}


	//
	// enum_field_types => string
	//
	static std::map<enum_field_types, const char*> m2string = {
		{ MYSQL_TYPE_TINY, "TINYINT" },
		{ MYSQL_TYPE_LONG, "INT" },
		{ MYSQL_TYPE_LONGLONG, "BIGINT" },
		{ MYSQL_TYPE_FLOAT, "FLOAT" },
		{ MYSQL_TYPE_VAR_STRING, "VARCHAR(128)" },
		{ MYSQL_TYPE_BLOB, "BLOB" },
		{ MYSQL_TYPE_LONG_BLOB, "LONGBLOB" },
	};
	const char* fieldstring(enum_field_types type) {
		auto i = m2string.find(type);
		assert(i != m2string.end());
		return m2string[type];
	}


	//
	// fetch row to Entity::Value
	//
	static std::map<enum_field_types, std::function<void(const char* org_name, Entity*, std::string, bool)>> m2value = {
		{ MYSQL_TYPE_TINY, [](const char* org_name, Entity* entity, std::string s, bool is_unsigned) {	// bool
			entity->setValue(org_name, std::stoi(s) != 0);
		}},
		{ MYSQL_TYPE_LONG, [](const char* org_name, Entity* entity, std::string s, bool is_unsigned) { 	// integer
			entity->setValue(org_name, std::stol(s));
		}},
		{ MYSQL_TYPE_LONGLONG, [](const char* org_name, Entity* entity, std::string s, bool is_unsigned) { // integer
			entity->setValue(org_name, std::stoll(s));
		}},
		{ MYSQL_TYPE_FLOAT, [](const char* org_name, Entity* entity, std::string s, bool is_unsigned) { // float
			entity->setValue(org_name, std::stof(s));
		}},
		{ MYSQL_TYPE_VAR_STRING, [](const char* org_name, Entity* entity, std::string s, bool is_unsigned) { // string
			entity->setValue(org_name, s.c_str());
		}},
		{ MYSQL_TYPE_BLOB, [](const char* org_name, Entity* entity, std::string s, bool is_unsigned) { // string
			entity->setValue(org_name, s.c_str());
		}},
		{ MYSQL_TYPE_LONG_BLOB, [](const char* org_name, Entity* entity, std::string s, bool is_unsigned) { // string
			entity->setValue(org_name, s.c_str());
		}},
	};
	void fieldvalue(enum_field_types type, const char* org_name, Entity* entity, char* row, bool is_unsigned) {
		auto i = m2value.find(type);
		assert(i != m2value.end());
		m2value[type](org_name, entity, row, is_unsigned);
	}
}

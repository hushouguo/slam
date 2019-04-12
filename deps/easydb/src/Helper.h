/*
 * \file: Helper.h
 * \brief: Created by hushouguo at 14:37:32 Apr 12 2019
 */
 
#ifndef __HELPER_H__
#define __HELPER_H__

namespace db {
	//
	// type of field => Entity::Value::Type
	Entity::Value::Type convert(enum_field_types type);

	//
	// Entity::Value::Type => type of field
	enum_field_types convert(const Entity::Value& value);

	//
	// enum_field_types => string
	const char* fieldstring(enum_field_types type);

	//
	// fetch row to Entity::Value
	void fieldvalue(enum_field_types type, Entity::Value& value, char* row, bool is_unsigned);

}

#endif

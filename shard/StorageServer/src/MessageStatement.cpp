/*
 * \file: MessageStatement.cpp
 * \brief: Created by hushouguo at 10:07:22 May 08 2019
 */

#include "common.h"
#include "mysql/MySQLResult.h"
#include "mysql/MySQLStatement.h"
#include "mysql/MySQL.h"
#include "MessageStatement.h"

BEGIN_NAMESPACE_SLAM {
	bool DecodeField(const Message& message, const FieldDescriptor* field, const Reflection* ref,
		const std::function<bool(const Message& message, const Reflection* ref, const FieldDescriptor* field)>& func) {	
		return func(message, ref, field);
	}	
	bool DecodeDescriptor(const Message& message, const Descriptor* descriptor, const Reflection* ref,
		const std::function<bool(const Message& message, const Reflection* ref, const FieldDescriptor* field)>& func) {
		int field_count = descriptor->field_count();
		for (int i = 0; i < field_count; ++i) {
			const FieldDescriptor* field = descriptor->field(i);
			if (!DecodeField(message, field, ref, func)) {
				Error("DecodeField: %s for message:%s failure", field->name().c_str(), message.GetTypeName().c_str());
				return false;
			}
		}
		return true;
	}	
	bool DecodeMessage(const Message& message, const Descriptor* descriptor, const Reflection* ref, 
		const std::function<bool(const Message& message, const Reflection* ref, const FieldDescriptor* field)>& func) {
		try {
			if (!DecodeDescriptor(message, descriptor, ref, func)) {
				Error << "DecodeDescriptor failure for message: " << message.GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "DecodeDescriptor exception:%s", e.what());
		}
		return true;
	}


	//
	// SQL statement
	bool GetFieldStatement(const DatabaseFieldDescriptor& descriptor, std::ostringstream& sql) {
		switch (descriptor.type) {
			case MYSQL_TYPE_TINY: sql << "TINYINT"; break;
			case MYSQL_TYPE_LONG: sql << "INT"; break;
			case MYSQL_TYPE_LONGLONG: sql << "BIGINT"; break;
			case MYSQL_TYPE_FLOAT: sql << "FLOAT"; break;
			case MYSQL_TYPE_DOUBLE: sql << "DOUBLE"; break;
			case MYSQL_TYPE_VAR_STRING: sql << "VARCHAR(" << descriptor.length << ")"; break;
			case MYSQL_TYPE_BLOB: sql << "BLOB"; break;
			case MYSQL_TYPE_LONG_BLOB: sql << "LONGBLOB"; break;
			default: CHECK_RETURN(false, false, "unknown MYSQL type: %d", descriptor.type);
		}
		if (IS_UNSIGNED(descriptor.flags)) { sql << " UNSIGNED"; }
		if (IS_NOT_NULL(descriptor.flags)) { sql << " NOT NULL"; }
		return true;
	}

	//
	// FieldType of protobuf::Message to FieldType of MySQL
	static std::map<FieldDescriptor::CppType, DatabaseFieldDescriptor> __msg2field = {
		{ CPPTYPE_INT32,	{MYSQL_TYPE_LONG, NOT_NULL_FLAG, 4} },// TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
		{ CPPTYPE_UINT32, 	{MYSQL_TYPE_LONG, NOT_NULL_FLAG | UNSIGNED_FLAG, 4} },// TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
		{ CPPTYPE_INT64, 	{MYSQL_TYPE_LONGLONG, NOT_NULL_FLAG, 8} },// TYPE_UINT32, TYPE_FIXED32
		{ CPPTYPE_UINT64,	{MYSQL_TYPE_LONGLONG, NOT_NULL_FLAG | UNSIGNED_FLAG, 8} },// TYPE_UINT64, TYPE_FIXED64
		{ CPPTYPE_FLOAT,	{MYSQL_TYPE_FLOAT, NOT_NULL_FLAG, 4} },// TYPE_FLOAT
		{ CPPTYPE_DOUBLE,	{MYSQL_TYPE_DOUBLE, NOT_NULL_FLAG, 8} },// TYPE_DOUBLE
		{ CPPTYPE_BOOL, 	{MYSQL_TYPE_TINY, NOT_NULL_FLAG | UNSIGNED_FLAG, 1} },// TYPE_BOOL
		{ CPPTYPE_ENUM, 	{MYSQL_TYPE_LONG, NOT_NULL_FLAG, 4} },// TYPE_ENUM
		{ CPPTYPE_STRING,	{MYSQL_TYPE_VAR_STRING, NOT_NULL_FLAG, 0} },// TYPE_STRING, TYPE_BYTES
		{ CPPTYPE_MESSAGE,	{MYSQL_TYPE_LONGBLOB, NOT_NULL_FLAG | BLOB_FLAG, 0} }// TYPE_MESSAGE, TYPE_GROUP
	};

	void ExtendField(const Message& message, const FieldDescriptor* field, const Reflection* ref, DatabaseFieldDescriptor& descriptor) {
		if (descriptor.type == MYSQL_TYPE_VAR_STRING) {
			enum_field_types newtype = descriptor.type;
			size_t newlen = 0;
			if (ref->HasField(message, field)) {
				std::string value = ref->GetString(message, field);
				if (value.length() < MYSQL_VARCHAR_UTF8_MAXSIZE) { newlen = value.length() / 32 + 32; }
				else if (value.length() < (u16)(-1)) {
					newtype = MYSQL_TYPE_BLOB;
					newlen = (u16)(-1);
				}
				else {
					newtype = MYSQL_TYPE_LONGBLOB;
					newlen = (u32)(-1);
				}
			}
			else {
				newlen = 32;
			}
			if (descriptor.type != newtype || newlen > descriptor.length) {
				Debug("field: %s from (%d,%ld) to (%d,%ld)", field->name().c_str(), 
					descriptor.type, descriptor.length,
					newtype, newlen);
				descriptor.type = newtype;
				descriptor.length = newlen;
			}
		}
	}



	//
	// add new field
	bool AddField(MySQL* mysql, std::string table, const std::string& field_name, const DatabaseFieldDescriptor& field) {
		if (field_name.compare("id")) {
			std::ostringstream sql;
			sql << "ALTER TABLE `" << table << "` ADD `" << field_name << "` ";
			bool rc = GetFieldStatement(field, sql);
			CHECK_RETURN(rc, false, "table: %s, field type: %d(%d,%d)", table.c_str(), field.type, field.flags, field.length);
			Debug << "add field: " << sql.str();
			return mysql->runCommand(sql.str());
		}
		return true;
	}
	
	//
	// alter field	
	bool AlterField(MySQL* mysql, std::string table, const std::string& field_name, const DatabaseFieldDescriptor& field) {
		if (field_name.compare("id")) {
			std::ostringstream sql;
			sql << "ALTER TABLE `" << table << "` MODIFY `" << field_name << "` ";
			bool rc = GetFieldStatement(field, sql);
			CHECK_RETURN(rc, false, "table: %s, field type: %d(%d,%d)", table.c_str(), field.type, field.flags, field.length);
			Debug << "alter field: " << sql.str();
			return mysql->runCommand(sql.str());
		}
		return true;
	}

	//
	// create table by Message
	bool CreateTable(MySQL* mysql, std::string table, const FieldSet& fieldSet) {
		std::ostringstream sql;
		sql << "CREATE TABLE IF NOT EXISTS `" << table << "`(";
		sql << "`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY"; // `id` is reserved
		for (auto& i : fieldSet) {
			if (i.first.compare("id")) {
				sql << ", `" << i.first << "` ";
				rc = GetFieldStatement(i.second, sql);
				CHECK_RETURN(rc, false, "field: %s, type:%d(%d,%d) error", i.first, i.second.type, i.second.flags, i.second.length);
			}
		}
		sql << ") ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8";
		bool rc = mysql->runCommand(sql.str());
		CHECK_RETURN(rc, 0, "run sql: %s error", sql.str().c_str());
		Debug << "create table: " << table << ", sql: " << sql.str();
		return rc;
	}


	//
	// MessageStatement
	//
	
	MessageStatement::MessageStatement(MySQL* mysql) {
		this->_dbhandler = mysql;
		//
		// load table
		std::set<std::string> tables;
		tables.clear();
		bool rc = this->_dbhandler->loadTable("%", tables);
		CHECK_PANIC(rc, "load table from database error");

		//
		// load fields decription
		for (auto& table : tables) {
			rc = this->loadField(table);
			CHECK_PANIC(rc, "load field: %s error", table.c_str());
		}

		this->DumpFieldDescriptor();
	}

	// 
	// field descriptor
	bool MessageStatement::LoadField(std::string table) {
		std::ostringstream sql;
		sql << "SELECT * FROM `" << table.c_str() << "` LIMIT 1";
		MySQLResult* result = this->_dbhandler->runQuery(sql.str());
		CHECK_RETURN(result, false, "runQuery: %s error", sql.str().c_str());

		u32 fieldNumber = result->fieldNumber();
		MYSQL_FIELD* fields = result->fetchField();

		FieldSet& fieldSet = this->_tables[table];
		for (u32 i = 0; i < fieldNumber; ++i) {
			const MYSQL_FIELD& field = fields[i];
			DatabaseFieldDescriptor& fieldDescriptor = fieldSet[field.org_name];
			fieldDescriptor.type = field.type;
			fieldDescriptor.flags = field.flags;
			fieldDescriptor.length = field.length;
		}

		SafeDelete(result);
		return true;
	}

	void MessageStatement::DumpFieldDescriptor() {
		Debug << "DumpFieldDescriptor";
		for (auto& i : this->_tables) {
			Debug << "\ttable: " << i.first;
			for (auto& v : i.second) {
				Debug << "\t\tfield: " << v.first << ", length: " << v.second.length;
			}
		}
	}

	bool GetFieldSet(const Message& message, const Reflection* ref, const FieldDescriptor* field, FieldSet& fieldSet) {
		CHECK_RETURN(FindOrNull(__msg2field, field->cpp_type()), false, "unhandled cpp_type: %d", field->cpp_type());
		if (ref->HasField(message, field)) {
			if (field->is_repeated()) {
				//TODO:
			}
			else {
				fieldSet[field->name()] = __msg2field[field->cpp_type()];
				ExtendField(message, field, ref, fieldSet[field->name()]);
			}
		}
		return true;
	}
	
	bool ScrapeStatement(const Message& message, const Reflection* ref, const FieldDescriptor* field, std::ostringstream& sql_fields, std::ostringstream& sql_values) {
		if (!ref->HasField(message, field)) { return true; }//TODO:
		if (sql_fields.rdbuf()->in_avail()) { sql_fields << ","; }
		if (sql_values.rdbuf()->in_avail()) { sql_values << ","; }
		sql_fields << field->name();
		switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(CPPTYPE, METHOD_TYPE, VALUE_TYPE)	\
			case google::protobuf::FieldDescriptor::CPPTYPE_##CPPTYPE: \
				if (true) {\
					VALUE_TYPE value = ref->Get##METHOD_TYPE(message, field);\
					sql_values << value;\
				} break;\
			CASE_FIELD_TYPE(INT32, Int32, int32_t);
			CASE_FIELD_TYPE(INT64, Int64, int64_t);
			CASE_FIELD_TYPE(UINT32, UInt32, uint32_t);
			CASE_FIELD_TYPE(UINT64, UInt64, uint64_t);
			CASE_FIELD_TYPE(DOUBLE, Double, double);
			CASE_FIELD_TYPE(FLOAT, Float, float);
			CASE_FIELD_TYPE(BOOL, Bool, bool);
			CASE_FIELD_TYPE(ENUM, EnumValue, int);
#undef CASE_FIELD_TYPE			
			case google::protobuf::FieldDescriptor::CPPTYPE_STRING: { // TYPE_STRING, TYPE_BYTES
				std::string value = ref->GetString(message, field);
				sql_values << "'" << value << "'";
			} break;							
			case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: { // TYPE_MESSAGE, TYPE_GROUP					
				const Message& submessage = ref->GetMessage(message, field, nullptr);
				std::string value;
				bool rc = submessage.SerializeToString(&value);
				CHECK_RETURN(rc, false, "field: %s SerializeToString: %s error", field->name().c_str(), submessage.GetTypeName().c_str());
				sql_values << "'" << value << "'";
			} break;
			default: CHECK_RETURN(false, false, "illegal field->cpp_type: %d,%s", field->cpp_type(), field->name().c_str());
		}
		return true;
	}
	
	bool MessageStatement::CreateMessage(std::string table, const Message* message, u64* insertid) {
		std::ostringstream sql_fields, sql_values;
		if (this->_tables.find(table) == this->_tables.end()) {
			//
			// create new table
			FieldSet& fieldSet = this->_tables[table];
			bool rc = DecodeMessage(*message, message->GetDescriptor(), message->GetReflection(), 
				[&fieldSet, &sql_fields, &sql_values](const Message& message, const Reflection* ref, const FieldDescriptor* field)->bool {
					return GetFieldSet(message, ref, field, fieldSet) 
						&& ScrapeStatement(message, ref, field, sql_fields, sql_values);
			});
			CHECK_RETURN(rc, false, "DecodeMessage to table: %s failure", table.c_str());			
#if true		
			Debug << "Message: " << message.GetTypeName();
			for (auto& i : fieldSet) {
				Debug << "\t" << i.first << " => " << i.second.type << "," << i.second.flags << "," << i.second.length;
			}
#endif
			rc = CreateTable(this->_dbhandler, table, fieldSet);
			CHECK_RETURN(rc, false, "create table: %s, message: %s error", table.c_str(), message->GetTypeName().c_str());
		}
		else {
			//
			// alter table
			FieldSet fieldSet;
			bool rc = DecodeMessage(*message, message->GetDescriptor(), message->GetReflection(),
				[&fieldSet, &sql_fields, &sql_values](const Message& message, const Reflection* ref, const FieldDescriptor* field)->bool {
					return GetFieldSet(message, ref, field, fieldSet) 
						&& ScrapeStatement(message, ref, field, sql_fields, sql_values);
			});
			CHECK_RETURN(rc, false, "DecodeMessage to table: %s failure", table.c_str());
			
			const FieldSet& lastSet = this->_tables[table];
			for (auto& i : fieldSet) {
				if (lastSet.find(i.first) == lastSet) {
					rc = AddField(this->_dbhandler, table, i.second);
					CHECK_RETURN(rc, false, "add field: %s error", i.first);
					lastSet[i.first] = i.second;
				}
				else {
					const DatabaseFieldDescriptor& descriptor = lastSet[i.first];
					if (i.second.flags != descriptor.flags 
							|| i.second.type != descriptor.type || i.second.length != descriptor.length) {
						rc = AlterField(this->_dbhandler, table, i.second);
						CHECK_RETURN(rc, false, "alter field: %s error", i.first);
						lastSet[i.first] = i.second;
					}
				}
			}
		}

		//
		// insert entity to table
		std::ostringstream sql;
		sql << "INSERT INTO `" << table << "` (" << sql_fields.str() << ") VALUES (" << sql_values.str() << ")";
		bool rc = this->_dbhandler->runCommand(sql.str());
		CHECK_RETURN(rc, false, "run sql: %s error", sql.str().c_str());
		if (insertid) {
			*insertid = this->_dbhandler->insertId();
		}
		return true;
	}

	bool MessageStatement::RetrieveMessage(std::string table, u64 entityid, Message* message) {
		return true;
	}

	bool MessageStatement::UpdateMessage(std::string table, u64 entityid, const Message* message) {
#if false	
		std::ostringstream sql, sql_fields, sql_values;
 		CHECK_RETURN(this->UpdateTable(table, message, &sql_fields, &sql_values), false, "update table: %s error", table.c_str());
 		sql << "UPDATE `" << table << "` (" << sql_fields.str() << ") VALUES (" << sql_values.str() << ")";
		bool rc = this->_dbhandler->runCommand(sql.str());
		CHECK_RETURN(rc, 0, "run sql: %s error", sql.str().c_str());
 		//return entity->id() == 0 ? this->_dbhandler->insertId() : entity->id();
 		//TODO:
#endif 		
		return true;
	}
	
	bool MessageStatement::DeleteMessage(std::string table, u64 entityid) {
		std::ostringstream sql;
		sql << "DELETE FROM TABLE `" << table << "` WHERE id = " << entityid;
		return this->_dbhandler->runCommand(sql.str());
	}
}

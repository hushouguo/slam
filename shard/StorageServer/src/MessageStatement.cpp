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
	MessageStatement::MessageStatement(MySQL* mysql) {
		this->_dbhandler = mysql;
		//
		// load table
		std::set<std::string> tables;
		tables.clear();
		rc = this->_dbhandler->loadTable("%", tables);
		CHECK_PANIC(rc, "load table from database: %s error", database.c_str());

		//
		// load fields decription
		for (auto& table : tables) {
			rc = this->loadField(table);
			CHECK_PANIC(rc, "load field: %s error", table.c_str());
		}

		this->DumpFieldDescriptor();
	}

	bool MessageStatement::SerializeMessage(std::string table, const Message* message, u64* insertid) {
		
	}

	bool MessageStatement::UnserializeMessage(std::string table, u64 entityid, Message* message) {
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
		{ CPPTYPE_INT32,	{MYSQL_TYPE_LONG, NOT_NULL_FLAG, 4} },
		{ CPPTYPE_UINT32, 	{MYSQL_TYPE_LONG, NOT_NULL_FLAG | UNSIGNED_FLAG, 4} },
		{ CPPTYPE_INT64, 	{MYSQL_TYPE_LONGLONG, NOT_NULL_FLAG, 8} },
		{ CPPTYPE_UINT64,	{MYSQL_TYPE_LONGLONG, NOT_NULL_FLAG | UNSIGNED_FLAG, 8} },
		{ CPPTYPE_FLOAT,	{MYSQL_TYPE_FLOAT, NOT_NULL_FLAG, 4} },
		{ CPPTYPE_DOUBLE,	{MYSQL_TYPE_DOUBLE, NOT_NULL_FLAG, 8} },
		{ CPPTYPE_BOOL, 	{MYSQL_TYPE_TINY, NOT_NULL_FLAG | UNSIGNED_FLAG, 1} },
		{ CPPTYPE_ENUM, 	{MYSQL_TYPE_LONG, NOT_NULL_FLAG, 4} },
		{ CPPTYPE_STRING,	{MYSQL_TYPE_VAR_STRING, NOT_NULL_FLAG, 0} },
		{ CPPTYPE_MESSAGE,	{MYSQL_TYPE_LONGBLOB, NOT_NULL_FLAG | BLOB_FLAG, 0} }
	};

	void ExtendField(const Message& message, const FieldDescriptor* field, const Reflection* ref, DatabaseFieldDescriptor& descriptor) {
		if (descriptor.type == MYSQL_TYPE_VAR_STRING) {
			std::string value = ref->GetString(message, field);
			if (value.length() < MYSQL_VARCHAR_UTF8_MAXSIZE) { descriptor.length = descriptor.length / 32 + 32; }
			else if (value.length() < (u16)(-1)) {
				descriptor.type = MYSQL_TYPE_BLOB;
				descriptor.length = (u16)(-1);
			}
			else {
				descriptor.type = MYSQL_TYPE_LONGBLOB;
				descriptor.length = (u32)(-1);
			}
		}
	}

	//
	// Decode protobuf::Message to FieldSet	
	bool DecodeField(const Message& message, const FieldDescriptor* field, const Reflection* ref, FieldSet& fieldSet) {
		if (field->is_repeated()) {
			auto& descriptor = fieldSet[field->name()];
			descriptor.type = MYSQL_TYPE_LONGBLOB;
			descriptor.flags = NOT_NULL_FLAG | BLOB_FLAG;
			descriptor.length = (u32)(-1);
		}
		else {
			CHECK_RETURN(FindOrNull(__msg2field, field->cpp_type()), false, "unhandled cpp_type: %d", field->cpp_type());
			DatabaseFieldDescriptor& descriptor = __msg2field[field->cpp_type()];
			ExtendField(message, field, ref, descriptor);
			fieldSet[field->name()] = __msg2field[field->cpp_type()];
		}
		return true;
	}

	bool DecodeDescriptor(const Message& message, const Descriptor* descriptor, const Reflection* ref, FieldSet& fieldSet) {
		int field_count = descriptor->field_count();
		for (int i = 0; i < field_count; ++i) {
			const FieldDescriptor* field = descriptor->field(i);
			if (!DecodeField(message, field, ref, fieldSet)) {
				Error("DecodeField: %s for message:%s failure", field->name().c_str(), message.GetTypeName().c_str());
				return false;
			}
		}
		return true;
	}

	bool DecodeMessage(const Message& message, const Descriptor* descriptor, const Reflection* ref, FieldSet& fieldSet) {
		try {
			if (!DecodeDescriptor(message, descriptor, ref, fieldSet)) {
				Error << "DecodeDescriptor failure for message: " << message.GetTypeName();
				return false;
			}
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "DecodeDescriptor exception:%s", e.what());
		}
		return true;
	}

	bool MessageStatement::UpdateTable(std::string table, const Message* message) {
		//
		// create new table
		if (this->_tables.find(table) == this->_tables.end()) {
			FieldSet& fieldSet = this->_tables[table];
			bool rc = DecodeMessage(*message, message->GetDescriptor(), message->GetReflection(), fieldSet);
			CHECK_RETURN(rc, false, "DecodeMessage to table: %s failure", table.c_str());
#if true		
			Debug << "Message: " << message.GetTypeName();
			for (auto& i : fieldSet) {
				Debug << "\t" << i.first << " => " << i.second.type << "," << i.second.flags << "," << i.second.length;
			}
#endif
			rc = this->CreateTable(table, fieldSet);
			CHECK_RETURN(rc, false, "create table: %s, message: %s error", table.c_str(), message->GetTypeName().c_str());
			return rc;
		}

		//
		// alter table
		FieldSet fieldSet;
		bool rc = DecodeMessage(*message, message->GetDescriptor(), message->GetReflection(), fieldSet);
		CHECK_RETURN(rc, false, "DecodeMessage to table: %s failure", table.c_str());
		const FieldSet& lastSet = this->_tables[table];
		for (auto& i : fieldSet) {
			if (lastSet.find(i.first) == lastSet) {
				rc = this->AddField(table, i.second);
				CHECK_RETURN(rc, false, "add field: %s error", i.first);
			}
			else {
				const DatabaseFieldDescriptor& descriptor = lastSet[i.first];
				if (i.second.flags != descriptor.flags 
						|| i.second.type != descriptor.type || i.second.length != descriptor.length) {
					rc = this->AlterField(table, i.second);
					CHECK_RETURN(rc, false, "alter field: %s error", i.first);
				}
			}
		}

	}

	//
	// add new field
	bool MessageStatement::AddField(std::string table, const std::string& field_name, const DatabaseFieldDescriptor& field) {
		std::ostringstream sql;
		sql << "ALTER TABLE `" << table << "` ADD `" << field_name << "` ";
		bool rc = GetFieldStatement(field, sql);
		CHECK_RETURN(rc, false, "table: %s, field type: %d(%d,%d)", table.c_str(), field.type, field.flags, field.length);
		//Debug << "add field: " << sql.str();
		return this->_dbhandler->runCommand(sql.str());
	}
	
	//
	// alter field	
	bool MessageStatement::AlterField(std::string table, const std::string& field_name, const DatabaseFieldDescriptor& field) {
		std::ostringstream sql;
		sql << "ALTER TABLE `" << table << "` MODIFY `" << field_name << "` ";
		bool rc = GetFieldStatement(field, sql);
		CHECK_RETURN(rc, false, "table: %s, field type: %d(%d,%d)", table.c_str(), field.type, field.flags, field.length);
		//Debug << "alter field: " << sql.str();
		return this->_dbhandler->runCommand(sql.str());
	}

	//
	// create table by Message
	bool MessageStatement::CreateTable(std::string table, const FieldSet& fieldSet) {
		std::ostringstream sql;
		sql << "CREATE TABLE IF NOT EXISTS `" << table << "`(";
		sql << "`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY"; // `id` is reserved
		for (auto& i : fieldSet) {
			sql << ", `" << i.first << "` ";
			rc = GetFieldStatement(i.second, sql);
			CHECK_RETURN(rc, false, "field: %s, type:%d(%d,%d) error", i.first, i.second.type, i.second.flags, i.second.length);
		}
		sql << ") ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8";
		bool rc = this->_dbhandler->runCommand(sql.str());
		CHECK_RETURN(rc, 0, "run sql: %s error", sql.str().c_str());
		Debug << "create table: " << table << ", sql: " << sql.str();
		return rc;
	}

	// alter table by Message
	bool MessageStatement::AlterTable(std::string table, const Message* message) {
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
}

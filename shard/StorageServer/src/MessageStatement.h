/*
 * \file: MessageStatement.h
 * \brief: Created by hushouguo at 09:53:06 May 08 2019
 */
 
#ifndef __MESSAGESTATEMENT_H__
#define __MESSAGESTATEMENT_H__

#ifndef IS_PRI_KEY
#define IS_PRI_KEY(n)	((n) & PRI_KEY_FLAG)
#endif

#ifndef IS_UNI_KEY
#define IS_UNI_KEY(n)	((n) & UNIQUE_KEY_FLAG)
#endif

#ifndef IS_MUL_KEY
#define IS_MUL_KEY(n)	((n) & MULTIPLE_KEY_FLAG)
#endif

#ifndef IS_UNSIGNED
#define IS_UNSIGNED(n)	((n) & UNSIGNED_FLAG)
#endif

#ifndef IS_NOT_NULL
#define IS_NOT_NULL(n)	((n) & NOT_NULL_FLAG)
#endif

#define MYSQL_VARCHAR_ASCII_MAXSIZE		65532
#define MYSQL_VARCHAR_UTF8_MAXSIZE		21845
#define MYSQL_VARCHAR_GBK_MAXSIZE		32766

BEGIN_NAMESPACE_SLAM {
	struct DatabaseFieldDescriptor {
		enum_field_types type;
		u32 flags;
		size_t length;
	};
	
	using std::unordered_map<std::string, DatabaseFieldDescriptor> = FieldSet;
	using std::unordered_map<std::string, FieldSet> = TableSet;
	
	class MySQL;
	class MessageStatement {
		public:
			MessageStatement(MySQL* mysql);

		public:
			bool CreateMessage(std::string table, const Message* message, u64* insertid);
			bool RetrieveMessage(std::string table, u64 entityid, Message* message);
			bool UpdateMessage(std::string table, u64 entityid, const Message* message);
			bool DeleteMessage(std::string table, u64 entityid);

		private:
			MySQL* _dbhandler = nullptr;

		private:
			TableSet _tables;
			bool LoadField(std::string table);
			void DumpFieldDescriptor();
	};
}

#endif

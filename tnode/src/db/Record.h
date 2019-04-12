/*
 * \file: Record.h
 * \brief: Created by hushouguo at 17:43:03 Apr 11 2019
 */
 
#ifndef __RECORD_H__
#define __RECORD_H__

BEGIN_NAMESPACE_TNODE {
	class Table;
	class Record : public Entry<u64> {
		public:
			virtual ~Record() = 0;
			const char* getClassName() override { return "record"; }
			virtual Table* table() = 0;

		public:
			virtual bool serialize() = 0;
			virtual bool unserialize() = 0;

		public:
			virtual const luaT_Value& getValue(const char* name) = 0;
			virtual void setValue(const char* name, const luaT_Value& value) = 0;
	};

	struct RecordCreator {
		static Record* create(Table* table);
	};
}

#endif

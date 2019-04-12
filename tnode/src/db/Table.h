/*
 * \file: Table.h
 * \brief: Created by hushouguo at 17:42:29 Apr 11 2019
 */
 
#ifndef __TABLE_H__
#define __TABLE_H__

BEGIN_NAMESPACE_TNODE {
	class Database;
	class Table {
		public:
			virtual ~Table() = 0;
			virtual Database* database() = 0;

		public:
				
	};
}

#endif

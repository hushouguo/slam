/*
 * \file: slam_db_table.h
 * \brief: Created by hushouguo at 11:35:11 Aug 22 2019
 */
 
#ifndef __SLAM_DB_TABLE_H__
#define __SLAM_DB_TABLE_H__

struct slam_db_table_s {
    slam_db_t* db;
    const char* table_name;
};

typedef struct slam_db_table_s slam_db_table_t;

extern slam_db_table_t* slam_db_table_new(slam_db_t* db, const char* table_name);
extern void slam_db_table_delete(slam_db_table_t* db_table);

#endif

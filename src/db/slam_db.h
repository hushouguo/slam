/*
 * \file: slam_db.h
 * \brief: Created by hushouguo at 17:30:55 Aug 21 2019
 */
 
#ifndef __SLAM_DB_H__
#define __SLAM_DB_H__

struct slam_db_s {
	slam_db_conf_t* conf;
	MYSQL mysqlhandle;
};

typedef struct slam_db_s slam_db_t;

extern slam_db_t* slam_db_new();
extern void slam_db_delete(slam_db_t* db);

extern bool slam_db_connect(slam_db_t* db, const char*, const char*, const char*, int, const char*);
extern bool slam_db_execute(slam_db_t* db, const char* sql);
extern slam_db_result_t* slam_db_query(slam_db_t* db, const char* sql);
extern uint64_t slam_db_insert_id(slam_db_t* db);

#endif

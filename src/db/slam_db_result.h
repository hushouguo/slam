/*
 * \file: slam_db_result.h
 * \brief: Created by hushouguo at 10:23:47 Aug 22 2019
 */
 
#ifndef __SLAM_DB_RESULT_H__
#define __SLAM_DB_RESULT_H__

struct slam_db_result_s {
	MYSQL_RES* res;
};

typedef struct slam_db_result_s slam_db_result_t;

extern slam_db_result_t* slam_db_result_new(MYSQL_RES* res);
extern void slam_db_result_delete(slam_db_result_t* result);

extern MYSQL_ROW slam_db_result_fetch_row(slam_db_result_t* result);
extern MYSQL_FIELD* slam_db_result_fetch_field(slam_db_result_t* result);
extern unsigned long* slam_db_result_fetch_length(slam_db_result_t* result);

extern uint64_t slam_db_result_row_number(slam_db_result_t* result);
extern uint64_t slam_db_result_field_number(slam_db_result_t* result);

#endif

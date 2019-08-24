/*
 * \file: slam_db_result.c
 * \brief: Created by hushouguo at 10:27:11 Aug 22 2019
 */

#include "slam.h"

slam_db_result_t* slam_db_result_new(MYSQL_RES* res) {
	slam_db_result_t* result = (slam_db_result_t *) slam_malloc(sizeof(slam_db_result_t));
	result->res = res;
	return result;
}

void slam_db_result_delete(slam_db_result_t* result) {
	mysql_free_result(result->res);
	slam_free(result);
}

MYSQL_ROW slam_db_result_fetch_row(slam_db_result_t* result) {
	return mysql_fetch_row(result->res);
}

MYSQL_FIELD* slam_db_result_fetch_field(slam_db_result_t* result) {
	return mysql_fetch_fields(result->res);
}

unsigned long* slam_db_result_fetch_length(slam_db_result_t* result) {
    return mysql_fetch_lengths(result->res);
}

uint64_t slam_db_result_row_number(slam_db_result_t* result) {
	return mysql_num_rows(result->res);
}

uint64_t slam_db_result_field_number(slam_db_result_t* result) {
	return mysql_num_fields(result->res);
}


/*
 * \file: slam_db_table.c
 * \brief: Created by hushouguo at 11:35:15 Aug 22 2019
 */

#include "slam.h"

slam_db_table_t* slam_db_table_new(slam_db_t* db, const char* table_name) {
    slam_db_table_t* db_table = (slam_db_table_t *) slam_malloc(sizeof(slam_db_table_t));
    db_table->db = db;
    db_table->table_name = strdup(table_name);
    return db_table;
}

void slam_db_table_delete(slam_db_table_t* db_table) {
    slam_free(db_table->table_name);
    slam_free(db_table);
}


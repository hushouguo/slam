/*
 * \file: slam_db.c
 * \brief: Created by hushouguo at 17:34:54 Aug 21 2019
 */

#include "slam.h"

#define MYSQL_CONNECT_TIMEOUT		10
#define MYSQL_WAIT_TIMEOUT			604800
#define MYSQL_INTERACTIVE_TIMEOUT	604800

slam_db_t* slam_db_new() {
    slam_db_t* db = (slam_db_t *) slam_malloc(sizeof(slam_db_t));
    db->conf = nullptr;
    mysql_init(&db->mysqlhandle);
	return db;
}

void slam_db_delete(slam_db_t* db) {
    mysql_close(&db->mysqlhandle);
    if (db->conf) {
        slam_db_conf_delete(db->conf);
    }
    slam_free(db);    
}

bool slam_db_connect(slam_db_t* db, const char* host, const char* username, const char* passwd, int port, const char* database) {
    if (true) { // connect timeout
        uint32_t seconds = MYSQL_CONNECT_TIMEOUT;
        if (mysql_options(&db->mysqlhandle, MYSQL_OPT_CONNECT_TIMEOUT, (const char *) &seconds)) {
            CHECK_RETURN(false, false, "mysql options error: %s", mysql_error(&db->mysqlhandle));
        }
    }

    if (true) { // enable reconnect
        my_bool reconnect = true;
        if (mysql_options(&db->mysqlhandle, MYSQL_OPT_RECONNECT, &reconnect)) {
            CHECK_RETURN(false, false, "mysql options error: %s", mysql_error(&db->mysqlhandle));
        }
    }

    if (true) { // wait timeout, default 8 hours
        char sql[960];
        snprintf(sql, sizeof(sql), "SET WAIT_TIMEOUT = %d", MYSQL_WAIT_TIMEOUT);
        if (mysql_options(&db->mysqlhandle, MYSQL_INIT_COMMAND, sql)) {
            CHECK_RETURN(false, false, "mysql options error: %s", mysql_error(&db->mysqlhandle));
        }
    }

    if (true) { // interactive timeout, default 8 hours
        char sql[960];
        snprintf(sql, sizeof(sql), "SET INTERACTIVE_TIMEOUT = %d", MYSQL_INTERACTIVE_TIMEOUT);
        if (mysql_options(&db->mysqlhandle, MYSQL_INIT_COMMAND, sql)) {
            CHECK_RETURN(false, false, "mysql options error: %s", mysql_error(&db->mysqlhandle));
        }
    }

    if (true) { // report data truncation
        my_bool report = true;          
        if (mysql_options(&db->mysqlhandle, MYSQL_REPORT_DATA_TRUNCATION, (const char *) &report)) {
            CHECK_RETURN(false, false, "mysql options error: %s", mysql_error(&db->mysqlhandle));
        }
    }

    if (!mysql_real_connect(
                &db->mysqlhandle, 
                host,
                username,
                passwd,
                database,
                port, nullptr, 0)) {
        CHECK_RETURN(false, false, "mysql connect error: %s", mysql_error(&db->mysqlhandle));
    }

    if (db->conf) {
        slam_db_conf_delete(db->conf);
    }
    db->conf = slam_db_conf_new(host, username, passwd, port);

    return slam_db_execute(db, "SET NAMES UTF8");
}

bool slam_db_execute(slam_db_t* db, const char* sql) {
    int rc = mysql_real_query(&db->mysqlhandle, sql, strlen(sql));
    CHECK_RETURN(rc == 0, false, "mysql query error: %s, %d", mysql_error(&db->mysqlhandle), rc);
    return true;
}

slam_db_result_t* slam_db_query(slam_db_t* db, const char* sql) {
    int rc = mysql_real_query(&db->mysqlhandle, sql, strlen(sql));
    CHECK_RETURN(rc == 0, nullptr, "mysql query error: %s, %d", mysql_error(&db->mysqlhandle), rc);
    MYSQL_RES* res = mysql_store_result(&db->mysqlhandle);
    CHECK_RETURN(res, nullptr, "mysql store result: %s", mysql_error(&db->mysqlhandle));
    return slam_db_result_new(res);
}

uint64_t slam_db_insert_id(slam_db_t* db) {
    return mysql_insert_id(&db->mysqlhandle);
}


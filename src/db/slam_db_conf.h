/*
 * \file: slam_db_conf.h
 * \brief: Created by hushouguo at 17:35:12 Aug 21 2019
 */
 
#ifndef __SLAM_DB_CONF_H__
#define __SLAM_DB_CONF_H__

struct slam_db_conf_s {
	const char* host;
	const char* username;
	const char* password;
	int port;
};

typedef struct slam_db_conf_s slam_db_conf_t;

extern slam_db_conf_t* slam_db_conf_new(const char*, const char*, const char*, int);
extern void slam_db_conf_delete(slam_db_conf_t* conf);

#endif

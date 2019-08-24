/*
 * \file: slam_db_conf.c
 * \brief: Created by hushouguo at 17:36:51 Aug 21 2019
 */

#include "slam.h"

slam_db_conf_t* slam_db_conf_new(const char* host, const char* username, const char* password, int port) {
	slam_db_conf_t* conf = (slam_db_conf_t *) slam_malloc(sizeof(slam_db_conf_t));
	conf->host = strdup(host);
	conf->username = strdup(username);
	conf->password = strdup(password);
	conf->port = port;
	return conf;
}

void slam_db_conf_delete(slam_db_conf_t* conf) {
	slam_free(conf->host);
	slam_free(conf->username);
	slam_free(conf->password);
	slam_free(conf);
}


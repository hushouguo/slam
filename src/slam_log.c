/*
 * \file: slam_log.c
 * \brief: Created by hushouguo at 17:04:44 Aug 07 2019
 */

#include "slam.h"

slam_log_t* slam_log_new(const char* filename) {
    int fd = open(filename, O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP);
    CHECK_RETURN(fd >= 0, nullptr, "open file: %s error: %d, %s", filename, errno, errstring(errno));
    slam_log_t* log = (slam_log_t *) slam_malloc(sizeof(slam_log_t));
    log->fd = fd;
    log->filename = strdup(filename);
    return log;
}

void slam_log_delete(slam_log_t* log) {
    slam_close(log->fd);
    slam_free(log->filename);
    slam_free(log);
}


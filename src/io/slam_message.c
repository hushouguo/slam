/*
 * \file: slam_message.c
 * \brief: Created by hushouguo at 10:50:47 Aug 02 2019
 */

#include "slam.h"

slam_message_t* slam_message_new(SOCKET fd, int type, size_t bufsize) {
    slam_message_t* message = (slam_message_t *) slam_malloc(sizeof(slam_message_t) + bufsize);
    message->fd = fd;
    message->type = type;
    message->bufsize = bufsize;
    message->bytesize = 0;
    return message;
}

void slam_message_delete(slam_message_t* message) {
    slam_free(message);
}


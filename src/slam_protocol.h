/*
 * \file: slam_protocol.h
 * \brief: Created by hushouguo at 10:42:00 Jul 26 2019
 */
 
#ifndef __SLAM_PROTOCOL_H__
#define __SLAM_PROTOCOL_H__

/* decode socket->rbuffer to lua's table */
extern bool slam_protocol_decode(slam_runnable_t* runnable, const slam_socket_t* socket);

/* encode lua's table to socket->wbuffer */
extern bool slam_protocol_encode(slam_runnable_t* runnable, slam_socket_t* socket);

#endif

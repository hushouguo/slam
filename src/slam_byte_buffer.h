/*
 * \file: slam_byte_buffer.h
 * \brief: Created by hushouguo at 16:47:16 Jul 23 2019
 */
 
#ifndef __SLAM_BYTE_BUFFER_H__
#define __SLAM_BYTE_BUFFER_H__

typedef struct slam_byte_buffer_s slam_byte_buffer_t;

extern slam_byte_buffer_t * slam_byte_buffer_new(size_t initsize);
extern void slam_byte_buffer_delete(slam_byte_buffer_t * byte_buffer);

extern size_t slam_byte_buffer_read(slam_byte_buffer_t * byte_buffer, void* buf, size_t bufsize);
extern void slam_byte_buffer_write(slam_byte_buffer_t * byte_buffer, const void* buf, size_t bufsize);

extern size_t slam_byte_buffer_size(slam_byte_buffer_t * byte_buffer);

extern void slam_byte_buffer_clear(slam_byte_buffer_t * byte_buffer);
extern void slam_byte_buffer_reset(slam_byte_buffer_t * byte_buffer);

extern byte_t* slam_byte_buffer_readbuffer(slam_byte_buffer_t * byte_buffer);
extern byte_t* slam_byte_buffer_writebuffer(slam_byte_buffer_t * byte_buffer, size_t needsize);

extern void slam_byte_buffer_readlength(slam_byte_buffer_t * byte_buffer, size_t len);
extern void slam_byte_buffer_writelength(slam_byte_buffer_t * byte_buffer, size_t len);

#endif

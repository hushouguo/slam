/*
 * \file: slam_byte_buffer.c
 * \brief: Created by hushouguo at 17:13:30 Jul 24 2019
 */

#include "slam.h"

#define BUFFER_CHUNK_SIZE   0x200

struct slam_byte_buffer_s {
	byte_t * bytes;
	size_t size, capacity;
	size_t rlength, wlength;
};

size_t slam_byte_buffer_blank(slam_byte_buffer_t * byte_buffer) {
	assert(byte_buffer->capacity >= byte_buffer->wlength);
	return byte_buffer->capacity - byte_buffer->wlength;
}

void slam_byte_buffer_reserve(slam_byte_buffer_t * byte_buffer, size_t newsize) {
	if (slam_byte_buffer_blank(byte_buffer) > newsize) {
		return;
	}

	if (newsize < BUFFER_CHUNK_SIZE) {
		newsize = BUFFER_CHUNK_SIZE;
	}

	newsize += byte_buffer->capacity;	

	byte_buffer->bytes = (byte_t *) realloc(byte_buffer->bytes, newsize);
	byte_buffer->capacity = newsize;
}

void slam_byte_buffer_shrink(slam_byte_buffer_t * byte_buffer) {
	if (slam_byte_buffer_size(byte_buffer) == 0) {
		slam_byte_buffer_reset(byte_buffer);
	}
}

slam_byte_buffer_t * slam_byte_buffer_new(size_t initsize) {
	slam_byte_buffer_t * byte_buffer = (slam_byte_buffer_t *) slam_malloc(sizeof(slam_byte_buffer_t));
	byte_buffer->bytes = NULL;
	byte_buffer->size = byte_buffer->capacity = 0;
	byte_buffer->rlength = byte_buffer->wlength = 0;
	slam_byte_buffer_reserve(byte_buffer, initsize);
	return byte_buffer;
}

void slam_byte_buffer_delete(slam_byte_buffer_t * byte_buffer) {
	slam_free(byte_buffer->bytes);
	slam_free(byte_buffer);
}

size_t slam_byte_buffer_read(slam_byte_buffer_t * byte_buffer, void* buf, size_t bufsize) {
	size_t readsize = slam_byte_buffer_size(byte_buffer);
	if (readsize > bufsize) {
		readsize = bufsize;
	}
	
	memcpy(buf, slam_byte_buffer_readbuffer(byte_buffer), readsize);
	slam_byte_buffer_readlength(byte_buffer, readsize);

	return readsize;
}

void slam_byte_buffer_write(slam_byte_buffer_t * byte_buffer, const void* buf, size_t bufsize) {
	memcpy(slam_byte_buffer_writebuffer(byte_buffer, bufsize), buf, bufsize);
	slam_byte_buffer_writelength(byte_buffer, bufsize);
}

size_t slam_byte_buffer_size(slam_byte_buffer_t * byte_buffer) {
	assert(byte_buffer->wlength >= byte_buffer->rlength);
	return byte_buffer->wlength - byte_buffer->rlength;
}

void slam_byte_buffer_clear(slam_byte_buffer_t * byte_buffer) {
	slam_free(byte_buffer->bytes);	
	byte_buffer->size = byte_buffer->capacity = 0;
	byte_buffer->rlength = byte_buffer->wlength = 0;
}

void slam_byte_buffer_reset(slam_byte_buffer_t * byte_buffer) {
	byte_buffer->rlength = byte_buffer->wlength = 0;
}

byte_t* slam_byte_buffer_readbuffer(slam_byte_buffer_t * byte_buffer) {
	return & byte_buffer->bytes[byte_buffer->rlength];
}

byte_t* slam_byte_buffer_writebuffer(slam_byte_buffer_t * byte_buffer, size_t needsize) {
	slam_byte_buffer_reserve(byte_buffer, needsize);
	return & byte_buffer->bytes[byte_buffer->wlength];
}

void slam_byte_buffer_readlength(slam_byte_buffer_t * byte_buffer, size_t len) {
	byte_buffer->rlength += len;
	assert(byte_buffer->wlength >= byte_buffer->rlength);
	slam_byte_buffer_shrink(byte_buffer);
}

void slam_byte_buffer_writelength(slam_byte_buffer_t * byte_buffer, size_t len) {
	byte_buffer->wlength += len;
	assert(byte_buffer->capacity >= byte_buffer->wlength);
}


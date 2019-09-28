
#ifndef __mr_buffer_h__
#define __mr_buffer_h__

#include <stdint.h>
#include <stdlib.h>

struct mr_buffer {
	int head_len;

	int pack_len;

	size_t read_size;
	size_t read_offset;
	struct mr_buffer_node *read_head;
	struct mr_buffer_node *read_tail;

	int read_len;
	int read_cap;
	char* read_data;

	size_t write_size;
	// size_t write_offset;
	struct mr_buffer_node *write_head;
	struct mr_buffer_node *write_tail;

	int write_len;
	int write_cap;
	char* write_data;
};


#define mr_buffer_size(buffer) (buffer)->size

// static inline size_t mr_buffer_size(struct mr_buffer* buffer){
// 	return buffer->size;
// }

struct mr_buffer* mr_buffer_create(int head_len);
void mr_buffer_free(struct mr_buffer* buffer);
int mr_buffer_read_push(struct mr_buffer* buffer, char* msg, size_t len);
int mr_buffer_read_header(struct mr_buffer* buffer, size_t len);
int mr_buffer_read(struct mr_buffer* buffer, char* data, int len);
int mr_buffer_read_pack(struct mr_buffer* buffer);


int mr_buffer_write_push(struct mr_buffer* buffer, char* msg, size_t len);
int mr_buffer_write_pack(struct mr_buffer* buffer);

// int mr_buffer_write_pack(struct mr_buffer* buffer, char* data, size_t len);


#endif
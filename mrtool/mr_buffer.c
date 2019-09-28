
#include "mr_buffer.h"
#include <assert.h>
#include <string.h>
#include "mr_config.h"

#define MIN_PACK 256


struct mr_buffer_node {
	char* msg;
	size_t sz;
	struct mr_buffer_node *next;
};


struct mr_buffer* mr_buffer_create(int head_len){
	struct mr_buffer* buffer = (struct mr_buffer*)MALLOC(sizeof(struct mr_buffer));
	memset(buffer, 0, sizeof(struct mr_buffer));

	// buffer->read_len = 0;
    buffer->read_cap = MIN_PACK;
	buffer->read_data = MALLOC(buffer->read_cap);

	// buffer->write_len = 0;
    buffer->write_cap = MIN_PACK;
	buffer->write_data = MALLOC(buffer->write_cap);

	if (head_len < 1)
		buffer->head_len = 2;
	else if (head_len > 4)
		buffer->head_len = 4;
	else
		buffer->head_len = head_len;

	return buffer;
}

void mr_buffer_free(struct mr_buffer* buffer){
	if (!buffer) return;
		
	struct mr_buffer_node* bnode;
	while(buffer->read_head != NULL){
		bnode = buffer->read_head;
		buffer->read_head = bnode->next;

		FREE(bnode->msg);
		FREE(bnode);
	}
	FREE(buffer->read_data);
	FREE(buffer->write_data);
	FREE(buffer);
}

int mr_buffer_read_push(struct mr_buffer* buffer, char* msg, size_t len){
	if (msg == NULL || len <= 0){
		return -1;
	}
	struct mr_buffer_node* bnode = (struct mr_buffer_node*)MALLOC(sizeof(struct mr_buffer_node));
	bnode->msg = (char*)MALLOC(len);
	memcpy(bnode->msg, msg, len);
	bnode->sz = len;
	bnode->next = NULL;

	if (buffer->read_head == NULL) {
		assert(buffer->read_tail == NULL);
		buffer->read_head = buffer->read_tail = bnode;
	} else {
		buffer->read_tail->next = bnode;
		buffer->read_tail = bnode;
	}
	buffer->read_size += len;
	return (int)buffer->read_size;
}

static inline void mr_buffer_pop_buffer_node(struct mr_buffer* buffer){
	struct mr_buffer_node* bnode = buffer->read_head;
	buffer->read_size -= bnode->sz - buffer->read_offset;
	buffer->read_offset = 0;
	buffer->read_head = bnode->next;
	if (buffer->read_head == NULL) {
		buffer->read_tail = NULL;
	}
	FREE(bnode->msg);
	FREE(bnode);
}

int mr_buffer_read_header(struct mr_buffer* buffer, size_t len) {
	if (len > 4 || len < 1) return -2;
	if (len > buffer->read_size) return -1;
	int sz = 0;
	struct mr_buffer_node * bnode = buffer->read_head;
	const uint8_t* ptr = (const uint8_t*)bnode->msg + buffer->read_offset;
	size_t i = 0;
	for (; i < len; i++) {
		sz <<= 8;
		sz |= *ptr;
		buffer->read_offset++;
		if (bnode->sz - buffer->read_offset == 0){
			mr_buffer_pop_buffer_node(buffer);
			if(i < len){
				break;
			}
			bnode = buffer->read_head;
			assert(bnode);
			ptr = (const uint8_t*)bnode->msg + buffer->read_offset;
		}else{
			ptr++;
		}
	}
	buffer->read_size -= len;
	return sz;
}

int mr_buffer_read(struct mr_buffer* buffer, char* data, int len) {
	if (data == NULL) return -2;
	if(len > buffer->read_size) return -1;

	struct mr_buffer_node* bnode = buffer->read_head;
	size_t size;
	size_t rd_len = len;
	do{
		size = bnode->sz - buffer->read_offset;
		if (rd_len >= size){
			memcpy(data, bnode->msg + buffer->read_offset, size);
			mr_buffer_pop_buffer_node(buffer);
			bnode = buffer->read_head;
			rd_len -= size;
			data += size;
		}else{
			memcpy(data, bnode->msg + buffer->read_offset, rd_len);
			buffer->read_offset += rd_len;
			buffer->read_size -= rd_len;
			rd_len = 0;
		}
	}while(rd_len > 0);
	return len;
}

static inline void mr_buffer_adjust_pack(struct mr_buffer* buffer, int len, int type)
{
	if (type == 0){
		if (len >= buffer->read_cap) {
	    	while(len >= buffer->read_cap) 
	    		buffer->read_cap *= 2;
	        
	        FREE(buffer->read_data);
	        buffer->read_data = MALLOC(buffer->read_cap);
	    }
	    else if (buffer->read_cap > MIN_PACK && buffer->read_cap > len*2) {
	        buffer->read_cap /= 2;
	        FREE(buffer->read_data);
	        buffer->read_data = MALLOC(buffer->read_cap);
	    }
	}else{
		if (len >= buffer->write_cap) {
	    	while(len >= buffer->write_cap) 
	    		buffer->write_cap *= 2;
	        
	        FREE(buffer->write_data);
	        buffer->write_data = MALLOC(buffer->write_cap);
	    }
	    else if (buffer->write_cap > MIN_PACK && buffer->write_cap > len*2) {
	        buffer->write_cap /= 2;
	        FREE(buffer->write_data);
	        buffer->write_data = MALLOC(buffer->write_cap);
	    }
	}
}

int mr_buffer_read_pack(struct mr_buffer* buffer) {
	int rlen = buffer->pack_len;
	if (rlen <= 0){
		rlen = mr_buffer_read_header(buffer, buffer->head_len);
		if (rlen <= 0){
			return -1;
		}
		buffer->pack_len = rlen;
	}
	if(rlen > buffer->read_size) return -1;
	buffer->pack_len = 0;

	buffer->read_len = rlen;
	mr_buffer_adjust_pack(buffer, rlen, 0);
	mr_buffer_read(buffer, buffer->read_data, rlen);
	return rlen;
}

//write////////////////
int mr_buffer_write_push(struct mr_buffer* buffer, char* msg, size_t len){
	if (msg == NULL || len <= 0){
		return -1;
	}
	struct mr_buffer_node* bnode = (struct mr_buffer_node*)MALLOC(sizeof(struct mr_buffer_node));
	bnode->msg = (char*)MALLOC(len);
	memcpy(bnode->msg, msg, len);
	bnode->sz = len;
	bnode->next = NULL;

	if (buffer->write_head == NULL) {
		assert(buffer->write_tail == NULL);
		buffer->write_head = buffer->write_tail = bnode;
	} else {
		buffer->write_tail->next = bnode;
		buffer->write_tail = bnode;
	}
	buffer->write_size += len;
	return (int)buffer->write_size;
}

int mr_buffer_write_pack(struct mr_buffer* buffer){
	int max_len = 0;
	int head_len = buffer->head_len;
	switch(head_len){
		case 1:
			max_len = 0xff;
		break;
		case 2:
			max_len = 0xffff;
		break;
		case 4:
			max_len = 0xffffffff;
		break;
		default:
			return -1;
		break;
	}

	size_t len = buffer->write_size;
	if (len > max_len){
		assert(0);
		return -1;
	}
	int wlen = head_len+(int)len;
	buffer->write_len = wlen;
	mr_buffer_adjust_pack(buffer, wlen, 1);

	char* data = buffer->write_data+head_len;

	struct mr_buffer_node* bnode;
	while(buffer->write_head != NULL){
		bnode = buffer->write_head;
		buffer->write_head = bnode->next;

		memcpy(data, bnode->msg, bnode->sz);
		data += bnode->sz;

		FREE(bnode->msg);
		FREE(bnode);
	}
	buffer->write_tail = NULL;
	buffer->write_size = 0;

	char* pdata = buffer->write_data;
	int i = head_len-1;
	for (; i >= 0; i--) {
		*(pdata+i) = len & 0xff;
		len >>= 8;
	}
	return buffer->write_len;
}

// int mr_buffer_write_pack(struct mr_buffer* buffer, char* data, size_t len) {
// 	int max_len = 0;
// 	int head_len = buffer->head_len;
// 	switch(head_len){
// 		case 1:
// 			max_len = 0xff;
// 		break;
// 		case 2:
// 			max_len = 0xffff;
// 		break;
// 		case 4:
// 			max_len = 0xffffffff;
// 		break;
// 		default:
// 			return -1;
// 		break;
// 	}
// 	if (len > max_len){
// 		assert(0);
// 		return -1;
// 	}

// 	int wlen = head_len+(int)len;
// 	buffer->write_len = wlen;
// 	assert(data != buffer->write_data);
// 	mr_buffer_adjust_pack(buffer, wlen, 1);
// 	memcpy(buffer->write_data+head_len, data, len);

// 	char* pdata = buffer->write_data;
// 	int i = head_len-1;
// 	for (; i >= 0; i--) {
// 		*(pdata+i) = len & 0xff;
// 		len >>= 8;
// 	}
// 	return buffer->write_len;
// }

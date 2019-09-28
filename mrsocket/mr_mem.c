
#include "mr_mem.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include "win/winport.h"
#include "win/atomic.h"
#else
#include "atomic.h"
#endif

#define MEMORY_ALLOCTAG 0x20190429
#define MEMORY_FREETAG 0x0badf00d

// #define MEMORY_CHECK

struct mem_cookie {
	uint32_t uid;
	uint32_t dogtag;
};

#define PREFIX_SIZE sizeof(struct mem_cookie)

static size_t _used_memory = 0;
static size_t _memory_block = 0;

static uint32_t _uid = 0;
static uint32_t _max_uid = 0;
static uint32_t* uid_mems = NULL;
static uint32_t* check_mems = NULL;

inline static void* fill_prefix(char* ptr, size_t size) {
	struct mem_cookie *p = (struct mem_cookie *)(ptr + size - sizeof(struct mem_cookie));
	uint32_t uid = ATOM_INC(&_uid);
	memcpy(&p->uid, &uid, sizeof(uid));
	if (uid < _max_uid){
		uid_mems[uid] = (uint32_t)size;
		if (check_mems[uid]){
			printf("fill_prefix check_mems uid=%d\n", uid);
		}
	}
	// if (size >= 1024){
	// 	printf("fill_prefix uid =%d, size=%dkb %db\n", uid, size >> 10, size%1024);
	// }else{
	// 	printf("fill_prefix uid =%d, size=%db\n", uid, size);
	// }
	// if (size >= 1024*3){
	// 	printf("fill_prefix big mem uid =%d, size=%dkb %db\n", uid, size >> 10, size%1024);
	// }

	uint32_t dogtag = MEMORY_ALLOCTAG;
	memcpy(&p->dogtag, &dogtag, sizeof(dogtag));

	ATOM_ADD(&_used_memory, (int)size);
	ATOM_INC(&_memory_block);
	*((size_t*)ptr) = size;
	return ptr+sizeof(size_t);
}

inline static void* clean_prefix(char* ptr) {
	ptr -= sizeof(size_t);
	size_t size = *((size_t*)ptr);
	struct mem_cookie *p = (struct mem_cookie *)(ptr + size - sizeof(struct mem_cookie));
	uint32_t uid;
	memcpy(&uid, &p->uid, sizeof(uid));
	if (uid < _max_uid){
		uid_mems[uid] = 0;
		if (check_mems[uid]) {
			printf("clean_prefix check_mems uid=%d\n", uid);
		}
	}
	uint32_t dogtag;
	memcpy(&dogtag, &p->dogtag, sizeof(dogtag));
	if (dogtag == MEMORY_FREETAG) {
		fprintf(stderr, "xmalloc: double free in :%d\n", uid);
	}
	assert(dogtag == MEMORY_ALLOCTAG);
	dogtag = MEMORY_FREETAG;
	memcpy(&p->dogtag, &dogtag, sizeof(dogtag));

	ATOM_SUB(&_used_memory, (int)size);
	ATOM_DEC(&_memory_block);
	return ptr;
}

void * mr_mem_malloc(size_t sz){
	if (_max_uid == 0){
		return malloc(sz);
	}
	size_t size = sz + PREFIX_SIZE+sizeof(size_t);
	void* ptr = malloc(size);
	return fill_prefix(ptr, size);
}

void mr_mem_free(void *ptr){
	if (ptr == NULL) return;
	if (_max_uid == 0){
		free(ptr);
		return;
	}
	void* rawptr = clean_prefix(ptr);
	free(rawptr);
}

void mr_mem_info(void){
	if (_max_uid > 0){
		uint32_t uid = 0;
		 for (; uid < _max_uid; ++uid){
		 	if(uid_mems[uid]){
		 		printf("mr_mem_info uid=%d, mem=%d \n", uid, uid_mems[uid]);
		 	}
		 }
	}
	printf("mr_mem_info used_memory=%ldkb, memory_block=%ld \n", _used_memory>>10, _memory_block);
}

void mr_mem_detect(uint32_t max_uid){
	assert(_max_uid == 0);
	_max_uid = max_uid;
	uid_mems = (uint32_t*)malloc(sizeof(uint32_t)*max_uid);
	memset(uid_mems, 0, sizeof(uint32_t)*max_uid);
	check_mems = (uint32_t*)malloc(sizeof(uint32_t)*max_uid);
	memset(check_mems, 0, sizeof(uint32_t)*max_uid);
}

void mr_mem_check(uint32_t uid){
	check_mems[uid] = 1;
}


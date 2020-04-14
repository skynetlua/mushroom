
#ifndef __mr_mem_h__
#define __mr_mem_h__

#include <stdint.h>
#include <stddef.h>

void * mr_mem_malloc(size_t sz);
void mr_mem_free(void *ptr);


struct mr_mem_info {
	uint32_t uid;
	uint32_t mem;
	struct mem_info* next;
};

struct mr_mem_info* mr_mem_info(void);
void mr_mem_detect(uint32_t max_uid);
void mr_mem_check(uint32_t uid);
struct mr_mem_info* mr_mem_check_info(void);

size_t mr_get_used_memory(void);
size_t mr_get_memory_block(void);


#endif
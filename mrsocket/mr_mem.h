
#ifndef __mr_mem_h__
#define __mr_mem_h__

#include <stdint.h>
#include <stddef.h>

void * mr_mem_malloc(size_t sz);
void mr_mem_free(void *ptr);
void mr_mem_info(void);

void mr_mem_detect(uint32_t max_uid);
void mr_mem_check(uint32_t uid);

#endif
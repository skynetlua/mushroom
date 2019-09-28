#ifndef mr_code_h
#define mr_code_h

#include <stdint.h>

#ifndef DATA_BIG_ENDIAN
    #ifdef _BIG_ENDIAN_
        #if _BIG_ENDIAN_
            #define DATA_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef DATA_BIG_ENDIAN
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
            #define DATA_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef DATA_BIG_ENDIAN
        #define DATA_BIG_ENDIAN  0
    #endif
#endif


static inline char *mr_encode8u(char *p, unsigned char c){
	*(unsigned char*)p++ = c;
	return p;
}

static inline const char *mr_decode8u(const char *p, unsigned char *c){
	*c = *(unsigned char*)p++;
	return p;
}

static inline char *mr_encode16u(char *p, unsigned short w){
#if DATA_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (w & 255);
	*(unsigned char*)(p + 1) = (w >> 8);
#else
	*(unsigned short*)(p) = w;
#endif
	p += 2;
	return p;
}

static inline const char *mr_decode16u(const char *p, unsigned short *w)
{
#if DATA_BIG_ENDIAN
	*w = *(const unsigned char*)(p + 1);
	*w = *(const unsigned char*)(p + 0) + (*w << 8);
#else
	*w = *(const unsigned short*)p;
#endif
	p += 2;
	return p;
}

static inline char *mr_encode32u(char *p, uint32_t l)
{
#if DATA_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (unsigned char)((l >>  0) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >>  8) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
#else
	*(uint32_t*)p = l;
#endif
	p += 4;
	return p;
}

static inline const char *mr_decode32u(const char *p, uint32_t *l)
{
#if DATA_BIG_ENDIAN
	*l = *(const unsigned char*)(p + 3);
	*l = *(const unsigned char*)(p + 2) + (*l << 8);
	*l = *(const unsigned char*)(p + 1) + (*l << 8);
	*l = *(const unsigned char*)(p + 0) + (*l << 8);
#else 
	*l = *(const uint32_t*)p;
#endif
	p += 4;
	return p;
}

static inline char *mr_encode64u(char *p, uint64_t l)
{
#if DATA_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (unsigned char)((l >>  0) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >>  8) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
	*(unsigned char*)(p + 0) = (unsigned char)((l >> 32) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >> 40) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 48) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 56) & 0xff);
#else
	*(uint64_t*)p = l;
#endif
	p += 8;
	return p;
}

static inline const char *mr_decode64u(const char *p, uint64_t *l)
{
#if DATA_BIG_ENDIAN
	*l = *(const unsigned char*)(p + 7);
	*l = *(const unsigned char*)(p + 6) + (*l << 8);
	*l = *(const unsigned char*)(p + 5) + (*l << 8);
	*l = *(const unsigned char*)(p + 4) + (*l << 8);
	*l = *(const unsigned char*)(p + 3) + (*l << 8);
	*l = *(const unsigned char*)(p + 2) + (*l << 8);
	*l = *(const unsigned char*)(p + 1) + (*l << 8);
	*l = *(const unsigned char*)(p + 0) + (*l << 8);
#else 
	*l = *(const uint64_t*)p;
#endif
	p += 8;
	return p;
}


#endif


#ifndef WIN_ATOMIC_H
#define WIN_ATOMIC_H

static inline bool __sync_bool_compare_and_swap(int* ptr, int oval, int nval){
	if(oval == InterlockedCompareExchange(ptr, nval, oval))
		return 1;
	
	return 0;
}

static inline int __sync_add_and_fetch(int* ptr, int n){
	InterlockedAdd(ptr, n);
	return *ptr;
}

static inline int __sync_sub_and_fetch(int* ptr, int n){
	InterlockedAdd(ptr, -n);
	return *ptr;
}

static inline int __sync_and_and_fetch(int* ptr, int n){
	InterlockedAnd(ptr, n);
	return *ptr;
}

#define ATOM_CAS(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
#define ATOM_CAS_POINTER(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
// #define ATOM_FINC(ptr) __sync_fetch_and_add(ptr, 1)
// #define ATOM_FDEC(ptr) __sync_fetch_and_sub(ptr, 1)

#define ATOM_INC(ptr) __sync_add_and_fetch(ptr, 1)
#define ATOM_DEC(ptr) __sync_sub_and_fetch(ptr, 1)
#define ATOM_ADD(ptr,n) __sync_add_and_fetch(ptr, n)
#define ATOM_SUB(ptr,n) __sync_sub_and_fetch(ptr, n)

#define ATOM_AND(ptr,n) __sync_and_and_fetch(ptr, n)


#endif

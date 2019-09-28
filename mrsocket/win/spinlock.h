#ifndef WIN_SPINLOCK_H
#define WIN_SPINLOCK_H

#define SPIN_INIT(q) spinlock_init(&(q)->lock);
#define SPIN_LOCK(q) spinlock_lock(&(q)->lock);
#define SPIN_UNLOCK(q) spinlock_unlock(&(q)->lock);
#define SPIN_DESTROY(q) spinlock_destroy(&(q)->lock);

#include "winport.h"
#include <windows.h>

struct spinlock {
    CRITICAL_SECTION lock;
};

static inline void
spinlock_init(struct spinlock *lock) {
	InitializeCriticalSectionAndSpinCount(&lock->lock, 4000);
}

static inline void
spinlock_lock(struct spinlock *lock) {
	EnterCriticalSection(&lock->lock);
}

static inline int
spinlock_trylock(struct spinlock *lock) {
	return TryEnterCriticalSection(&lock->lock);
}

static inline void
spinlock_unlock(struct spinlock *lock) {
    LeaveCriticalSection(&lock->lock);
}

static inline void
spinlock_destroy(struct spinlock *lock) {
	DeleteCriticalSection(&lock->lock);
}

// struct spinlock {
// 	pthread_mutex_t lock;
// };

// static inline void
// spinlock_init(struct spinlock *lock) {
// 	pthread_mutex_init(&lock->lock, NULL);
// }

// static inline void
// spinlock_lock(struct spinlock *lock) {
// 	pthread_mutex_lock(&lock->lock);
// }

// static inline int
// spinlock_trylock(struct spinlock *lock) {
// 	return pthread_mutex_trylock(&lock->lock) == 0;
// }

// static inline void
// spinlock_unlock(struct spinlock *lock) {
// 	pthread_mutex_unlock(&lock->lock);
// }

// static inline void
// spinlock_destroy(struct spinlock *lock) {
// 	pthread_mutex_destroy(&lock->lock);
// }


#endif

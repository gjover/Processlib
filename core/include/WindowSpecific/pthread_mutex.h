#ifndef __PTHREAD_MUTEX_H__
#define __PTHREAD_MUTEX_H__

#include <Windows.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/timeb.h>

typedef CRITICAL_SECTION pthread_mutex_t;
inline int pthread_mutex_lock(pthread_mutex_t *m)
{
	EnterCriticalSection(m);
	return 0;
}

inline int pthread_mutex_unlock(pthread_mutex_t *m)
{
	LeaveCriticalSection(m);
	return 0;
}
	
inline int pthread_mutex_trylock(pthread_mutex_t *m)
{
	return TryEnterCriticalSection(m) ? 0 : EBUSY; 
}

typedef int pthread_mutexattr_t;

inline int pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *a)
{
	(void) a;
	InitializeCriticalSection(m);
	
	return 0;
}

inline int pthread_mutex_destroy(pthread_mutex_t *m)
{
	DeleteCriticalSection(m);
	return 0;
}

#define PTHREAD_MUTEX_INITIALIZER {(PRTL_CRITICAL_SECTION_DEBUG)-1,-1,0,0,0,0}

struct timespec
{
	/* long long in windows is the same as long in unix for 64bit */
	long long tv_sec;
	long long tv_nsec;
};

inline unsigned long long _pthread_time_in_ms(void)
{
	struct __timeb64 tb;
	
	_ftime64(&tb);
	
	return tb.time * 1000 + tb.millitm;
}

inline unsigned long long _pthread_time_in_ms_from_timespec(const struct timespec *ts)
{
	unsigned long long t = ts->tv_sec * 1000;
	t += ts->tv_nsec / 1000000;

	return t;
}

inline unsigned long long _pthread_rel_time_in_ms(const struct timespec *ts)
{
	unsigned long long t1 = _pthread_time_in_ms_from_timespec(ts);
	unsigned long long t2 = _pthread_time_in_ms();
	
	/* Prevent underflow */
	if (t1 < t2) return 1;
	return t1 - t2;
}
#define ETIMEDOUT	110
inline int pthread_mutex_timedlock(pthread_mutex_t *m, struct timespec *ts)
{
	unsigned long long t, ct;
	
	struct _pthread_crit_t
	{
		void *debug;
		LONG count;
		LONG r_count;
		HANDLE owner;
		HANDLE sem;
		ULONG_PTR spin;
	};
	
	/* Try to lock it without waiting */
	if (!pthread_mutex_trylock(m)) return 0;
	
	ct = _pthread_time_in_ms();
	t = _pthread_time_in_ms_from_timespec(ts);
	
	while (1)
	{
		/* Have we waited long enough? */
		if (ct > t) return ETIMEDOUT;
		
		/* Wait on semaphore within critical section */
		WaitForSingleObject(((struct _pthread_crit_t *)m)->sem,DWORD(t - ct));
		
		/* Try to grab lock */
		if (!pthread_mutex_trylock(m)) return 0;
		
		/* Get current time */
		ct = _pthread_time_in_ms();
	}
}
#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_ERRORCHECK 1
#define PTHREAD_MUTEX_RECURSIVE 2
#define PTHREAD_MUTEX_DEFAULT 3
#define PTHREAD_MUTEX_SHARED 4
#define PTHREAD_MUTEX_PRIVATE 0
#define PTHREAD_PRIO_MULT 32

#define ENOTSUP		134

#define pthread_mutex_getprioceiling(M, P) ENOTSUP
#define pthread_mutex_setprioceiling(M, P) ENOTSUP

inline int pthread_mutexattr_init(pthread_mutexattr_t *a)
{
	*a = 0;
	return 0;
}

inline int pthread_mutexattr_destroy(pthread_mutexattr_t *a)
{
	(void) a;
	return 0;
}

inline int pthread_mutexattr_gettype(pthread_mutexattr_t *a, int *type)
{
	*type = *a & 3;

	return 0;
}

inline int pthread_mutexattr_settype(pthread_mutexattr_t *a, int type)
{
	if ((unsigned) type > 3) return EINVAL;
	*a &= ~3;
	*a |= type;
	
	return 0;
}

inline int pthread_mutexattr_getpshared(pthread_mutexattr_t *a, int *type)
{
	*type = *a & 4;
	
	return 0;
}

inline int pthread_mutexattr_setpshared(pthread_mutexattr_t * a, int type)
{
	if ((type & 4) != type) return EINVAL;
	
	*a &= ~4;
	*a |= type;
	
	return 0;
}

inline int pthread_mutexattr_getprotocol(pthread_mutexattr_t *a, int *type)
{
	*type = *a & (8 + 16);
	
	return 0;
}

inline int pthread_mutexattr_setprotocol(pthread_mutexattr_t *a, int type)
{
	if ((type & (8 + 16)) != 8 + 16) return EINVAL;
	
	*a &= ~(8 + 16);
	*a |= type;
	
	return 0;
}

inline int pthread_mutexattr_getprioceiling(pthread_mutexattr_t *a, int * prio)
{
	*prio = *a / PTHREAD_PRIO_MULT;
	return 0;
}

inline int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *a, int prio)
{
	*a &= (PTHREAD_PRIO_MULT - 1);
	*a += prio * PTHREAD_PRIO_MULT;
	
	return 0;
}

#endif

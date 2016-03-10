
#ifndef __TIMED_H__
#define __TIMED_H__

#include <sys/time.h>

typedef struct periodic_job_t {
	struct timeval freq;
}periodic_job_t;

void run_periodic_job(void (*per_func)(), periodic_job_t *job);

#endif /* __TIMED_H__ */

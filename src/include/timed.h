#ifndef __TIMED_H__
#define __TIMED_H__

#include <sys/time.h>

typedef struct periodic_job_t {
	struct timeval freq;
}periodic_job_t;

void run_periodic_job(void (*fun_job)(), periodic_job_t *job);

#endif /* __TIMED_H__ */

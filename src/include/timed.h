#ifndef __TIMED_H__
#define __TIMED_H__

#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>
#include <string.h>

#include "log.h"

typedef void (*fun_job)(void);

int to_timed_init_job(char *job_name, int frequency);
void to_timed_start_job(void);
void to_timed_stop_job(void);
void to_timed_run_periodic_job(fun_job do_job);

#endif /* __TIMED_H__ */

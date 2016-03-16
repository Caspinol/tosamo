#include "include/timed.h"

#define MAX_JOB_NAME 512

typedef struct periodic_job_t {
	char job_name[512];
	long freq;
	bool running;
}periodic_job_t;

static periodic_job_t job;

int to_timed_init_job(char * job_name, int frequency){

	size_t jobname_len = strlen(job_name);
	
	if(jobname_len > MAX_JOB_NAME){
		to_log_err("job name too long");
		return -1;
	}

	strncpy(job.job_name, job_name, jobname_len);
	job.freq = frequency * 1000;
	job.running = true;

	return 0;
}

void to_timed_stop_job(void){
	job.running = false;
}

/* It will run specified function periodically */
void to_timed_run_periodic_job(fun_job do_job, void *data){

	LOG_LEVEL0("Starting [%s] job every %d(s)", job.job_name, job.freq/1000);

	do{
		do_job(data);
		LOG_LEVEL3("Job [%s] finished", job.job_name);
	}while((poll(NULL, 0, job.freq) >= 0));
	
	to_log_warn("Periodic job [%s] stopped", job.job_name);
}

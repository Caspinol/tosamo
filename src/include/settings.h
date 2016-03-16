#ifndef __TK_CFG_H__
#define __TK_CFG_H__

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "lists.h"
#include "log.h"


#define IP_LEN 15
#define TAG_LEN 15
#define OBJ_PATH_LEN 256

typedef enum {
	MASTER,
	SLAVE
} running_mode_e;

typedef struct local_settings_t{
	
	running_mode_e running_mode;                /* Are we master or slave */
	char           my_ip[IP_LEN + 1];           /* IP we want to bind to */ 
	char           remote_ip[IP_LEN + 1];       /* Where do we send/recv the updates */
	char           port[IP_LEN + 1];

	char           **object_path;               /* File we want to syncronise */
	int            object_count;                /* How many files we need to keep track */
	
	char           tag[TAG_LEN + 1];            /* Pattern to mark the parts to sync */
	int            scan_frequency;              /* How often we send updates */
	int            log_level;                   
	bool           daemonize;                   /* Should we run as daemon */
	char           pid_file[OBJ_PATH_LEN];      /* Where do we store the pid file */
	
}local_settings_t;

/* The struct containing local settings is going to be global */
extern local_settings_t main_settings;

int to_parse_local_settings(char *);

#endif

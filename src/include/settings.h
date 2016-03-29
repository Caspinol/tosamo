#ifndef __TK_CFG_H__
#define __TK_CFG_H__

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h> /* For the ADDR string */

#ifdef __linux__
# include <linux/limits.h> /* For the MAX_PATH */
#elif __APPLE__
# include <sys/syslimits.h>
#else
# define PATH_MAX (4096)
#endif

#include "utils.h"
#include "lists.h"
#include "log.h"

#define INET_PORTSTRLEN  (5 + 1)
#define TAG_LEN          (15 + 1)

/* 
   MAX_PATH is 4K but the usage case of this for config files.
   Config files are in /etc, /usr/local/etc etc...
   So 4K is an overkill and I go here with 1K which is still more then 
   enough for most cases
*/
#define OBJ_PATH_LEN     (PATH_MAX / 4)

typedef enum {
	MASTER,
	SLAVE
} running_mode_e;

typedef struct local_settings_t{
	
	int            log_level;                   
	int            scan_frequency;                 /* How often we send updates */

	int            remote_ip_count;
	int            object_count;                   /* How many files we need to keep track */
	bool           daemonize;                      /* Should we run as daemon */

	char           (*remote_ip)[INET6_ADDRSTRLEN]; /* Where do we send/recv the updates */
	char           (*object_path)[OBJ_PATH_LEN];   /* File we want to syncronise */

	char           my_ip[INET6_ADDRSTRLEN];        /* IP we want to bind to */ 
	char           port[INET_PORTSTRLEN];
	
	char           tag[TAG_LEN];                   /* Pattern to mark the parts to sync */

	char           pid_file[OBJ_PATH_LEN];         /* Where do we store the pid file */

	running_mode_e running_mode;                   /* Are we master or slave */
	
}local_settings_t;

/* The struct containing local settings is going to be global */
extern local_settings_t main_settings;

int to_parse_local_settings(char *);
void to_cleanup_settings(void);

#endif

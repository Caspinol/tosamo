#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "include/settings.h"
#include "include/master.h"
#include "include/slave.h"
#include "include/utils.h"
#include "include/log.h"

#define VERSION "0.5.4"
#define PROGNAME "tosamo"
#define PROG_VER (PROGNAME " - " VERSION)

/* local definitions */
static void show_version();
static void show_usage(int);
static void handle_signal(int);

/* struct containing all program settings */
local_settings_t main_settings;

/* main function */
int main(int argc, char **argv){
	
	struct sigaction sa;
	char *tosamo_cfg = NULL;
	int argval; //parsing cli arguments
	pid_t pid;
	
	if(argc < 2){
		show_usage(EXIT_FAILURE);
	}

	/* Start the syslogging */
	to_log_start(PROG_VER);
	
	while ((argval = getopt(argc, argv, "dc:VvTh")) != EOF) {
		switch(argval){

		case 'd':
			main_settings.daemonize = true;
			break;
			
		case 'c':
			tosamo_cfg = optarg;
			break;
			
		case 'V':
			show_version();
			break;
			
		case 'v':
			verbose++;
			break;
			
		case 'h':
			show_usage(EXIT_SUCCESS);
			break;
			
		default:
			show_usage(EXIT_FAILURE);
			break;
		}
	}

	/* Start by parsing settings */
	if(to_parse_local_settings(tosamo_cfg) == RET_NOK) exit(EXIT_FAILURE);

	/* Update versbosity level to whatever is higher */
	if(main_settings.log_level > verbose) verbose = main_settings.log_level;

	LOG_LEVEL3("Setting up signal handlers");
	sa.sa_handler = &handle_signal;
	sa.sa_flags = SA_NOCLDWAIT;
	sigemptyset(&sa.sa_mask);
	
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		to_log_err("Could not handle INT signal"); //is it possible?
	}
	if (sigaction(SIGHUP, &sa, NULL) == -1) {
		to_log_err("Could not handle HUP signal"); //is it possible?
	}
	/* ignore the children and prevent them turning into zombies */  
	signal(SIGCHLD, SIG_IGN);

	if(main_settings.daemonize){
		pid = fork();
		
		/* error so no point continuing */
		if(pid < 0) exit(EXIT_FAILURE);

	}	

	
	
	if(main_settings.running_mode == MASTER){
		mstr_send_update();
	}else{
		slave_handle_updates();
	}
        
}

static void handle_signal(int signal){
	
	switch(signal){
	case SIGHUP:
		to_log_info("Got SIGHUP - so I'm terminating.\nSee ya!");
		break;
		
	case SIGINT:
		to_log_info("Inerrupted by user with SIGINT.\nSee ya!");
		break;
		
	default:
		to_log_err("Caught wrong signal. Don't know what to do...lets panic!!!");
		break;
	}
}

static void show_version(){
	
	fprintf(stdout, "\n");
	fprintf(stdout, "%s - partial config file syncroniser\n", PROGNAME);
	fprintf(stdout, "Version: %s\n", VERSION);

	exit(EXIT_SUCCESS);
}

static void show_usage(int status){
	FILE *output = status ? stderr : stdout;
	
	fprintf(output, "\n");
	fprintf(output, "%s - partial config file synchroniser\n", PROGNAME);
	fprintf(output, "Version: %s\n", VERSION);
	fprintf(output, "\nUsage: %s [options]\n", PROGNAME);
	fprintf(output, "Options:\n");
	fprintf(output, "\t\"-c\" - path to tosamo.cfg file\n");
	fprintf(output, "\t\"-d\" - run as daemon\n");
	fprintf(output, "\t\"-v\" - verbose output.\n\t\tMultiple \"v\" for increased logging.\n");
	fprintf(output, "\t\"-V\" - show version and exit.\n");
	fprintf(output, "\t\"-h\" - display this help menu.\n");

	exit(status);
}

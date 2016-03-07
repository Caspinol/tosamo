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

#define VERSION "0.5.3"
#define PROGNAME "tosamo"
#define PROG_VER (PROGNAME " - " VERSION)

/* local definitions */
static void show_version();
static void show_usage(int);
static void handle_signal(int);
static void cleanup();

static int argval; //parsing cli arguments  

static char *tosamo_cfg = NULL;
static int exit_status = EXIT_SUCCESS;

local_settings_t main_settings;

/* main function */
int main(int argc, char **argv){
	
	struct sigaction sa;
	
	if(argc < 2){
		show_usage(EXIT_FAILURE);
	}

	/* Start the syslogging */
	to_log_start(PROG_VER);
	
	while ((argval = getopt(argc, argv, "c:VvTh")) != EOF) {
		switch(argval){
        		
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

	/* Parse program settings */
	LOG_LEVEL1("Parsing settings file...");
	if(to_parse_local_settings(tosamo_cfg) == RET_NOK){
		exit_status = EXIT_FAILURE;
		goto EXIT;
	}

	LOG_LEVEL2("runnning mode -> [%s]",
		   (main_settings.running_mode == MASTER) ? "MASTER" : "SLAVE");
	LOG_LEVEL2("my_ip -> [%s]", main_settings.my_ip);
	LOG_LEVEL2("remote_ip -> [%s]", main_settings.remote_ip);
	LOG_LEVEL2("port -> [%s]", main_settings.port);
	LOG_LEVEL2("tag -> [%s]", main_settings.tag);
	LOG_LEVEL2("object_file -> [%s]", main_settings.object_path);

	/* Update versbosity level */
	if(main_settings.log_level > verbose) verbose = main_settings.log_level;

	LOG_LEVEL1("Setting up signal handlers");
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

	if(main_settings.running_mode == MASTER){
		mstr_send_update();
	}else{
		slave_handle_updates();
	}
	
 EXIT:
	/* need to tidy up */
	cleanup();
}

static void handle_signal(int signal){
	
	switch(signal){
	case SIGHUP:
		to_log_info("Got SIGHUP - so I'm terminating.\nSee ya!");
		cleanup();
		break;
		
	case SIGINT:
		to_log_info("Inerrupted by user with SIGINT.\nSee ya!");
		cleanup();
		break;
		
	default:
		to_log_err("Caught wrong signal. Don't know what to do...lets panic!!!");
		cleanup();
		break;
	}
}

static void cleanup(){
        
	to_log_close();
	exit(exit_status);
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
	fprintf(output, "\t\"-V\" - show version and exit.\n");
	fprintf(output, "\t\"-v\" - verbose output.\n\t\tMultiple \"v\" for increased logging.\n");
	fprintf(output, "\t\"-c\" - path to tosamo.cfg file\n");
	fprintf(output, "\t\"-h\" - display this help menu.\n");

	exit(status);
}

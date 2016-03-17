#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "include/settings.h"
#include "include/master.h"
#include "include/slave.h"
#include "include/utils.h"
#include "include/log.h"

#include "include/configure.h"

/* local definitions */
static void show_version();
static void show_usage(int);
static void handle_signal(int);

/* struct containing all program settings */
local_settings_t main_settings;

/* Controls state of the accept */
volatile static int running = 1;

/* main function */
int main(int argc, char **argv){
	
	struct sigaction     sa;
	char                 *tosamo_cfg = NULL;
	int                  argval; //parsing cli arguments
	int	             from_child[2] = {-1, -1}; /* for pipe() comms */
	
	if(argc < 2){
		show_usage(EXIT_FAILURE);
	}

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
	if(to_parse_local_settings(tosamo_cfg)) exit(EXIT_FAILURE);

	/* Update versbosity level to whatever is higher */
	if(main_settings.log_level > verbose) verbose = main_settings.log_level;

	/* Now init the syslog logging */
	to_log_start(PROG_VER, main_settings.daemonize);
	
	/* Time to daemonize */
	if(main_settings.daemonize){
		pid_t     pid;
		int       stat_loc;
		int       devnull;

		devnull = open("/dev/null", O_RDWR);
		if (devnull < 0) {
			to_log_err("Failed opening /dev/null: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
		dup2(devnull, STDIN_FILENO);
		close(devnull);

		/* pipe will help to confirm that daemonization was succesful */
		if(pipe(from_child) != 0){
			to_log_err("Failure opening pipe for child comms: [%s]\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		pid = fork();
		
		/* error so no point continuing */
		if(pid < 0) exit(EXIT_FAILURE);

		if(pid > 0){
			/* Parent */
			uint8_t retcode = 0;
			
			close(from_child[1]);

			/* Lets see if the child succedded */
			if((read(from_child[0], &retcode, 1) < 0)) retcode = 0;

			close(from_child[0]);

			if(!retcode){
				waitpid(pid, &stat_loc, WNOHANG);
				exit(EXIT_FAILURE);
			}

			to_log_info("All looks good so parent peace out!\n");
			exit(EXIT_SUCCESS);
		}

		/* Become session leader */
		setsid();

		/* Apply new permissions */
		umask(0);

		/* Set working directory */
		chdir("/");

	}	

	/* Install signal handlers */
	sa.sa_handler = &handle_signal;
	sa.sa_flags = SA_NOCLDWAIT;
	sigemptyset(&sa.sa_mask);
	
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	/* ignore the children and prevent them turning into zombies */  
	signal(SIGCHLD, SIG_IGN);


	/* Time to write the pid */
	FILE *fp = fopen(main_settings.pid_file, "w");
	if(fp){
		fprintf(fp, "%d\n", (int)getpid());
		fclose(fp);
	}else{
		to_log_err("Failed to create PID file: [%s]", strerror(errno));
		to_cleanup_settings();
		exit(EXIT_FAILURE);
	}	

	/* Let parent know that the child is ok */
	if (main_settings.daemonize) {

		close(from_child[0]);
		
		if (write(from_child[1], "\001", 1) < 0) {
			to_log_err("Failed to send OK to parent: [%s]", strerror(errno));
		}
		close(from_child[1]);
	}

	if(main_settings.running_mode == MASTER){
		master_monitor_files();
	}else{
		slave_handle_updates(&running);
	}
}

static void handle_signal(int signal){
	
	switch(signal){
	case SIGHUP:
		to_log_info("Got SIGHUP - so I'm terminating.");
		break;
		
	case SIGINT:
		to_log_info("Inerrupted by user with SIGINT.\nSee ya!");
		running = 0;
		to_cleanup_settings();
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

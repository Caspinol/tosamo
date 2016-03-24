#ifndef __LOG_H_
#define __LOG_H_

#ifdef DEBUG
 #define DINFO(MSG, ...) fprintf(stderr, "[INFO](%s:%d) " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__)
 #define DERROR(MSG, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " MSG "\n", __FILE__, __LINE__, \
				 strerror(errno), ##__VA_ARGS__)
 #define DWARN(MSG, ...) fprintf(stderr, "[WARN] (%s:%d) " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
 #define DINFO(MSG, ...)
 #define DERROR(MSG, ...)
 #define DWARN(MSG, ...)
#endif

#define LOG_LEVEL0(MSG, ...)					\
	do{							\
		to_log_info(MSG, ##__VA_ARGS__);		\
	}while(0);						\


#define LOG_LEVEL1(MSG, ...)					\
	do{							\
		if(verbose){					\
			to_log_info(MSG, ##__VA_ARGS__);	\
		}						\
	}while(0);						\

#define LOG_LEVEL2(MSG, ...)					\
	do{							\
		if(verbose > 1){				\
			to_log_info(MSG, ##__VA_ARGS__);	\
		}						\
	}while(0);						\

#define LOG_LEVEL3(MSG, ...)					\
	do{							\
		if(verbose > 2){				\
			to_log_info(MSG, ##__VA_ARGS__);	\
		}						\
	}while(0);						\
        
	
/* detailed logging */
extern int verbose; 

void to_log_start(char *progname, bool daemonize);
void to_log_err(const char *format, ...);
void to_log_info(const char *format, ...);
void to_log_warn(const char *format, ...);
void to_log_close();

#endif

#include "include/settings.h"

#define LINE 512 //should be more than enough for config line
#define KV_MAX_LEN LINE/2

typedef struct {
	char *keyword;
	bool is_found;
}allowed_keywords_t;

static void print_local_settings(void);
static int validate_key(char const *);
static int populate_main_settings(void);

/* Array of allowed keywords in settings file */
static allowed_keywords_t allowed_keywords[] = {
	{ "mode", false },
	{ "my_ip", false },
	{ "remote_ip", false },
       	{ "port", false },
	{ "tag", false },
	{ "object_file", false },
	{ "log_level", false },
	{ "pid_file", false },
	{ "scan_frequency", false },
	
	{ NULL, false }
};

/* What we have parsed from the config file */
static L_HEAD * settings = NULL; 

void print_local_settings(void){
	LOG_LEVEL2("runnning mode -> [%s]",
		   (main_settings.running_mode == MASTER) ? "MASTER" : "SLAVE");
	LOG_LEVEL2("my_ip -> [%s]", main_settings.my_ip);
	LOG_LEVEL2("port -> [%s]", main_settings.port);
	LOG_LEVEL2("tag -> [%s]", main_settings.tag);
	LOG_LEVEL2("daemon mode -> [%s]", main_settings.daemonize ? "TRUE" : "FALSE");
	LOG_LEVEL2("pid file path -> [%s]", main_settings.pid_file);
	LOG_LEVEL2("file scan freq -> [%d]", main_settings.scan_frequency);
	LOG_LEVEL2("Slaves:");
	for(int i = 0; i < main_settings.remote_ip_count; i++){
		LOG_LEVEL2("\t[%s]", main_settings.remote_ip[i]);
	}
}

int to_parse_local_settings(char *file){
	int r_status = -1;
	
	char line[LINE];

	settings = to_list_create(NULL, NULL);
	
	fprintf(stdout, "Parsing [%s] for settings\n", file);
	
	FILE *settings_file = fopen(file, "r");
	if(settings_file == NULL){
		fprintf(stderr, "Cannot read [%s]\n", file);
		goto CLEANUP;
	}
	
	while(fgets(line, LINE, settings_file) != NULL){
		
		char *p = line;

		while(isspace((int)*p)) p++;
		if(!*p || *p == '#' || *p == ';' || *p == '\n') continue;
		
		int i, j, eq;
		for(i = 0; i < LINE; i++){

			char key[KV_MAX_LEN] = {0},
				value[KV_MAX_LEN] = {0};
			
			if(line[i] == '='){
				//found the key...
				memcpy(key, line, i);
				key[i] = '\0';
				eq = i;
				to_str_trim(key);
				if(validate_key(key)){
					
					fprintf(stderr, "Skipping unknown keyword: [%s]\n", key);
					continue;
				}
				/* ...so there must be value */
				for(j=i; j<LINE; j++){
					/* check for termination or for comment marker '#' or ';' */
					if(line[j+1] == '\0' || line[j+1] == '\n' || line[j+1] == ';'){
						
						//end of line - len of key gives value
						memcpy(value, (line + eq + 1), (j - strlen(key)));
						value[j] = '\0';
						to_str_trim(value);
						break;
					}
				}
				KV_PAIR * kv_pair = to_kvpair_create(key, strlen(key), value, strlen(value));
				if(!kv_pair){
					goto CLEANUP;
				}
				
				to_list_push(settings, kv_pair);
				break;
			}
		}
	}

	if(populate_main_settings()){
		fprintf(stderr, "Failed to populate config struct\n");
		goto CLEANUP;
	}

	print_local_settings();
	r_status = 0;

 CLEANUP:
	to_list_destroy(settings);
	fclose(settings_file);
	return r_status;
}

static int validate_key(char const * key){
	int i;
        for(i = 0; allowed_keywords[i].keyword; i++){
		if(!strcmp(key, allowed_keywords[i].keyword)){
			return 0;
		}		
	}
	return -1;
}

static int populate_main_settings(void){

	KV_PAIR *pair = NULL;

	/* Required */
	pair = to_list_find(settings, "mode");
	if(!pair) return -1;
	main_settings.running_mode = (strncmp(pair->value, "master", strlen("master"))==0) ? MASTER : SLAVE;
	
	pair = to_list_find(settings, "my_ip");
	if(!pair) return -1;
	strncpy(main_settings.my_ip, pair->value, pair->vlen);
	
	pair = to_list_find(settings, "port");
	if(!pair) return -1;
	strncpy(main_settings.port, pair->value, pair->vlen);
	
	pair = to_list_find(settings, "tag");
	if(!pair) return -1;
	strncpy(main_settings.tag, pair->value, pair->vlen);

	pair = to_list_find(settings, "log_level");
	if(!pair) return -1;
	main_settings.log_level = atoi(pair->value);

	pair = to_list_find(settings, "scan_frequency");
	if(!pair) return -1;
	main_settings.scan_frequency = atoi(pair->value);

	pair = to_list_find(settings, "pid_file");
	if(!pair) return -1;
	strncpy(main_settings.pid_file, pair->value, pair->vlen);

	
	/* We also allow multiple slaves */
	main_settings.remote_ip_count = to_list_get_count(settings, "remote_ip");
	main_settings.remote_ip = malloc(main_settings.remote_ip_count * sizeof(*main_settings.remote_ip));
	if(!main_settings.remote_ip){
		fprintf(stderr, "No memory\n");
		return -1;
	}
	int obj_count = 0;
	do{

		pair = to_list_get(settings, "remote_ip");
		if(!pair) return -1;

		strcpy(main_settings.remote_ip[obj_count], pair->value);
		to_kvpair_destroy(pair);
		obj_count++;

	}while(to_list_peek(settings, "remote_ip") && obj_count < main_settings.remote_ip_count);

	
	/* We need to count how  many files we track */
	main_settings.object_count = to_list_get_count(settings, "object_file");
	main_settings.object_path = malloc(main_settings.object_count * sizeof(*main_settings.object_path));
	if(!main_settings.object_path){
		fprintf(stderr, "No memory\n");
		return -1;
	}


	obj_count = 0;
	do{

		pair = to_list_get(settings, "object_file");
		if(!pair) return -1;

		strcpy(main_settings.object_path[obj_count], pair->value);
		to_kvpair_destroy(pair);
		obj_count++;

	}while(to_list_peek(settings, "object_file") && obj_count < main_settings.object_count);
	
	return 0;
}

void to_cleanup_settings(void){

	free(main_settings.object_path);
	free(main_settings.remote_ip);

	main_settings.object_count = 0;
}

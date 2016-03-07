#include "include/settings.h"

#define LINE 512 //should be more than enough for config line
#define KV_MAX_LEN LINE/2

typedef struct {
	char *keyword;
	bool is_found;
}allowed_keywords_t;

static ret_code_e validate_key(char const *);
static ret_code_e populate_main_settings(void);

/* Array of allowed keywords in settings file */
static allowed_keywords_t allowed_keywords[] = {
	{ "mode", false },
	{ "my_ip", false },
	{ "remote_ip", false },
       	{ "port", false },
	{ "tag", false },
	{ "object_file", false },
	{ "log_level", false },
	
	{ NULL, false }
};

/* What we have parsed from the config file */
static L_HEAD * settings = NULL; 

void to_print_local_settings(void){
	LOG_LEVEL2("runnning mode -> [%s]",
		   (main_settings.running_mode == MASTER) ? "MASTER" : "SLAVE");
	LOG_LEVEL2("my_ip -> [%s]", main_settings.my_ip);
	LOG_LEVEL2("remote_ip -> [%s]", main_settings.remote_ip);
	LOG_LEVEL2("port -> [%s]", main_settings.port);
	LOG_LEVEL2("tag -> [%s]", main_settings.tag);
	LOG_LEVEL2("object_file -> [%s]", main_settings.object_path);
	LOG_LEVEL2("daemon mode -> [%s]", main_settings.daemonize ? "TRUE" : "FALSE");
}

ret_code_e to_parse_local_settings(char *file){
	ret_code_e r_status = RET_NOK;
	
	char line[LINE];

	settings = to_list_create();
	
	fprintf(stdout, "Parsing [%s] for settings", file);
	
	FILE *settings_file = fopen(file, "r");
	if(settings_file == NULL){
		fprintf(stderr, "Cannot read [%s]", file);
		goto CLEANUP;
	}
	
	while(fgets(line, LINE, settings_file) != NULL){
		char key[KV_MAX_LEN] = {0}, value[KV_MAX_LEN] = {0};
		if(line[0] == '#' ||line[0] == ';'|| line[0] == '\n'){
			continue;
		}
		int i, j, eq;
		for(i = 0; i < LINE; i++){
			if(line[i] == '='){
				//found the key...
				strncpy(key, line, i);
				eq = i;
				key[i] = '\0';
				to_str_trim(key);
				if(validate_key(key)){
					/* Dont bother continuing
					   Just cleanup and exit */
					fprintf(stderr, "Unknown keyword: [%s]", key);
					goto CLEANUP;
				}
				/* ...so there must be value */
				for(j=i; j<LINE; j++){
					/* check for termination or for comment marker '#' or ';' */
					if(line[j+1] == '\0' || line[j+1] == '\n' || line[j+1] == ';'){
						
						//end of line - len of key gives value
						strncpy(value, (line + eq + 1), (j - strlen(key)));
						value[j] = '\0';
						to_str_trim(value);
						break;
					}
				}
				break;
			}
		}

		KV_PAIR * kv_pair = to_kvpair_create(key, strlen(key), value, strlen(value));
		if(!kv_pair){
			goto CLEANUP;
		}

		to_list_push(settings, kv_pair);
	}

	if(populate_main_settings() == RET_NOK){
		fprintf(stderr, "Failed to populate config struct");
		goto CLEANUP;
	}
	
	r_status = RET_OK;

 CLEANUP:
	to_list_destroy(settings);
	fclose(settings_file);
	return r_status;
}

static ret_code_e validate_key(char const * key){
	int i;
        for(i = 0; allowed_keywords[i].keyword != NULL; i++){
		if(!strncmp(key, allowed_keywords[i].keyword, strlen(key))){
			return RET_OK;
		}		
	}
	return RET_NOK;
}

static ret_code_e populate_main_settings(void){

	KV_PAIR *pair = to_list_find(settings, "mode");
	if(!pair) return RET_NOK;
	main_settings.running_mode = (strncmp(pair->value, "master", strlen("master"))==0) ? MASTER : SLAVE;
	
	pair = to_list_find(settings, "my_ip");
	if(!pair) return RET_NOK;
	strncpy(main_settings.my_ip, pair->value, pair->vlen);
	
	pair = to_list_find(settings, "remote_ip");
	if(!pair) return RET_NOK;
	strncpy(main_settings.remote_ip, pair->value, pair->vlen);
	
	pair = to_list_find(settings, "port");
	if(!pair) return RET_NOK;
	strncpy(main_settings.port, pair->value, pair->vlen);
	
	pair = to_list_find(settings, "tag");
	if(!pair) return RET_NOK;
	strncpy(main_settings.tag, pair->value, pair->vlen);
	
	pair = to_list_find(settings, "object_file");
	if(!pair) return RET_NOK;
	strncpy(main_settings.object_path, pair->value, pair->vlen);
	
	pair = to_list_find(settings, "log_level");
	if(!pair) return RET_NOK;
	main_settings.log_level = atoi(pair->value);
	
	return RET_OK;
}

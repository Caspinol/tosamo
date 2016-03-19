#include "include/utils.h"

char *to_time_now(void){
	time_t rawtime;
	struct tm * timeinfo;
	
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return asctime (timeinfo);
}

char *to_str_reverse(char const *str, size_t len){
	char const *s = str;
	//fast forwaard till end of string
	while(*s!='\0') s++;
	char *r = malloc(1 + len * sizeof(char));
	if(r){
		while((s--) > str){
			*(r++) = *s;
		}
		*r='\0';
		return r-len;
	}
	return NULL;
}

void to_str_del_substr(char *str, const char *sub, size_t sublen){
	char *s = str;
	char *f = strstr(s, sub);
	
	if(f){
		while(s<f) s++;
		
		while(*(s+sublen) != '\0'){
			*s = *(s+sublen);
			s++;
		}
		*s='\0';
	}
}

char *to_itos(int i, char ret[]){
	sprintf(ret, "%d", i);
	return ret;
}

size_t to_get_filelen(FILE *f){
	fseek(f, 0, SEEK_END);
	size_t fs = ftell(f);
	rewind(f);
	return fs;
}

void to_str_trim(char * s) {
	char * str = s;

	if(!str) return;
	
	int len = strlen(str);
	if(len == 0) return;
	
	while(isspace(str[len - 1])){
		str[--len] = 0;
	}
	while(*str && isspace(*str)){
		++str, --len;
	}
	memmove(s, str, len + 1);
}

int to_is_big_endiann(){
	union{
		int i;
		char c[4];
	}t;
	t.i = 0x01000000;
	
	return t.c[0];
}

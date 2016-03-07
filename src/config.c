#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "include/log.h"
#include "include/utils.h"
#include "include/lists.h"
#include "include/config.h"

typedef enum {
	
	GOODTAG,
	NOOPENTAG,
	NOCLOSETAG
  
}TAG_RESULT;

static size_t file_to_buffer(FILE *, char *, size_t);
static L_HEAD * obj_buffer_chop(char *, int, char *, bool);

/* extern var */
int verbose = 0;

/* Reads the entire file into a memory */
static size_t file_to_buffer(FILE * f, char *whole_file, size_t file_size){
	size_t bytesRead;
	
	/* read it all */
	bytesRead = fread(whole_file, 1, file_size, f);
	if(bytesRead != file_size){
		to_log_err("Cannot read all bytes from [%s]", f);
		return -1;
	}
	whole_file[bytesRead] = '\0';
	
	return bytesRead;
}

static L_HEAD * obj_buffer_chop(char * obj_file_buf, int buflen, char * tag, bool whole){
	char *otag, /* open tag */
		*ctag, /* closing tag */
		*cop = obj_file_buf; /* current object ptr */
	int part_count = 1;
	char key[10];
	size_t ltag = strlen(tag); /* Length of the tag string */
	KV_PAIR *kv_pair = NULL;
	L_HEAD * file_parts = to_list_create();

	/* set the opening and closing tag patterns */
	char *open_tag_ptrn = tag;
	char *close_tag_ptrn = to_str_reverse(tag, ltag);

	/* find opening tag */
	otag = strstr(obj_file_buf, open_tag_ptrn);
	if(otag){
		do{
			if(whole){
				/* copy the file before the tag, excluding tag */
				kv_pair = to_kvpair_create("head", strlen("head"),
							   cop, ((otag-cop)));
				if(kv_pair){
					to_list_push(file_parts, kv_pair);
				}
			}
			/* continue from the opening tag */
			ctag = strstr(otag, close_tag_ptrn);
			if(ctag){
				to_itos(part_count++, key);
				cop = ctag;
				/* the value is the string between tags
				 * including the tags
				 */
				cop += ltag;
				kv_pair = to_kvpair_create(key, strlen(key), otag, cop - otag);

				to_list_push(file_parts, kv_pair);
			}else{
				to_log_err("Closing tag(%s) missing from file", close_tag_ptrn);
				free(close_tag_ptrn);
				to_list_destroy(file_parts);
				return NULL;
			}

			otag = strstr(ctag, open_tag_ptrn);
		}while(otag != NULL && (otag < obj_file_buf + buflen));
	}else{
		to_log_err("Opening tag(%s) missing from file", open_tag_ptrn);
		free(close_tag_ptrn);
		to_list_destroy(file_parts);
		return NULL;
	}

	if(whole){
		/* skip the opening tag */
		kv_pair = to_kvpair_create("ass", strlen("ass"), cop, strlen(cop));
		if(kv_pair){
			to_list_push(file_parts, kv_pair);
		}
	}

	free(close_tag_ptrn);
	return file_parts;
}

L_HEAD * obj_buffer_parse(char * buffer, int buflen, char *tag){

	return obj_buffer_chop(buffer, buflen, tag, false);
}

L_HEAD * obj_file_parse(char * file, char * tag, bool whole){
	FILE * obj_file;
	size_t file_size = 0;
	L_HEAD * tagged_parts_list = NULL;
	
	obj_file = fopen(file, "r+");
	if(obj_file == NULL){  
		to_log_err("Error opening file [%s]", file);
		return NULL;
	}
	
	file_size = to_get_filelen(obj_file);
	char obj_file_buf[file_size + 1];
	

	file_to_buffer(obj_file, obj_file_buf, file_size);

	tagged_parts_list = obj_buffer_chop(obj_file_buf, file_size, tag, whole);
	
	fclose(obj_file);
	return tagged_parts_list;
}

/* Replace parts in @local list with matching elements from @remote list */ 
int obj_file_replace_tagged_parts(L_HEAD * local, L_HEAD * remote){

	if(!remote || !local) return -1;
	if(remote->count == 0 || local->count == 0) return -1;
	
	/* We can use here nested loops or just extract whats needed */
	
	/* string representation of the part number 5 digit number is more than enough */
	char part_s[5+1] = { '1','\0' }; 
	int part_i = 2; /* part number as integer */
	
	do{
		KV_PAIR *kv = to_list_get(remote, part_s);
		to_list_replace(local, kv);

		to_itos(part_i++, part_s);
		
	}while(to_list_peek(remote, part_s));

	/* how many nodes were replaced */
	return part_i;
}

int obj_write_to_file(char const *file, L_HEAD const *head){
	FILE *obj_file;
	KV_PAIR *p;
	
	obj_file = fopen(file, "wb");
	if(obj_file == NULL){  
		to_log_err("Error opening file [%s]", file);
		return -1;
	}
	
	for(const L_NODE *n=head->node; n; n=n->next){
		p = n->data;
		
		fprintf(obj_file, "%s", p->value);
	}
	
	fclose(obj_file);
	return 0;
}

#ifndef __LISTS_H_
#define __LISTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "log.h"


typedef struct kv_pair{
	char * key;
	char * value;
	size_t vlen; /* Length of value */
}KV_PAIR;

typedef struct cf_node{
	void * data;
	struct cf_node * next;
}L_NODE;

typedef struct l_head{
	struct cf_node * node;
	int count; /* Number of elements in the list */
}L_HEAD;

KV_PAIR *to_kvpair_create(char *, size_t, char *, size_t);
void to_kvpair_destroy(KV_PAIR *);

L_HEAD *to_list_create();
void to_list_replace(L_HEAD *, void *);
void to_list_push(L_HEAD *, void *);
bool to_list_peek(L_HEAD *, void *);
void *to_list_find(L_HEAD *, void *);
void *to_list_get(L_HEAD *, void *);
size_t to_list_2_buf(L_HEAD *head, char **buffer);
void to_list_destroy(L_HEAD *);

#endif

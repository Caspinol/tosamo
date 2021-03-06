#ifndef __LISTS_H_
#define __LISTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "log.h"


typedef struct kv_pair{
	size_t vlen; /* Length of value */
	char * value;
	char * key;
}KV_PAIR;

typedef struct cf_node{
	void * data;
	struct cf_node * next;
}L_NODE;

typedef void (*list_data_delete)(void *data);
typedef bool (*list_data_compare)(void *c1, void *c2);

typedef struct l_head{
	int count; /* Number of elements in the list */
	list_data_delete data_del;
	list_data_compare data_cmp;
	struct cf_node * node;
}L_HEAD;

KV_PAIR *to_kvpair_create(char *, size_t, char *, size_t);
void to_kvpair_destroy(KV_PAIR *);

L_HEAD *to_list_create(list_data_delete data_del, list_data_compare data_cmp);
void to_list_replace(L_HEAD *, void *);
void to_list_push(L_HEAD *, void *);
bool to_list_peek(L_HEAD *, void *);
int to_list_get_count(L_HEAD *head, void *key);
void *to_list_find(L_HEAD *, void *);
void *to_list_get(L_HEAD *, void *);
size_t to_list_2_buf(L_HEAD *head, char **buffer);
void to_list_destroy(L_HEAD *);

#endif

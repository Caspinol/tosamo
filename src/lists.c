#include "include/lists.h"

KV_PAIR *to_kvpair_create(char *key, size_t klen, char *value, size_t vlen){
	KV_PAIR *b = malloc(sizeof(KV_PAIR));
	if(!b){
		to_log_err("Cannot malloc() the KV_PAIR - exiting");
		return NULL;
	}
	
	b->value = malloc(1 + vlen * sizeof(char));
	if(!b->value){
		to_log_err("Cannot malloc() the KV_PAIR->value - exiting");
		return NULL;
	}
	
	b->key = malloc(1 + klen * sizeof(char));
	if(!b->key){
		to_log_err("Cannot create key");
		return NULL;
	}
	
	strncpy(b->key, key, klen);
	b->key[klen] = '\0';
	strncpy(b->value, value, vlen);
	b->value[vlen] = '\0';
	
	b->vlen = vlen;
	return b;
}

void to_kvpair_destroy(KV_PAIR *pair){
	if(pair){
		free(pair->key);
		free(pair->value);
		free(pair);
	}
}

/* Some defaut implementation of compare and delete functions */
static void default_data_del(void *data){
	to_kvpair_destroy((KV_PAIR *)data);
}

static bool default_data_cmp(void *c1, void *c2){

	if(!c1 || !c2) return false;
	
	char * p1 = ((KV_PAIR *)c1)->key;
	char * p2 = (char *)c2;

	if(!strcmp(p1, p2)){
		return true;
	}
	return false;
}

/*
  New list instantiation with specified name
  
  @param name - name of the list (filename)

  @return h - pointer to a head of a list
  or NULL if unsuccessfull
 */
L_HEAD *to_list_create(list_data_delete data_del, list_data_compare data_cmp){
	L_HEAD *head = NULL;

	head = malloc(1 * sizeof(L_HEAD));
	if(!head) return NULL;
	
	head->node = calloc(1, sizeof(L_NODE));
	if(!head->node){
		free(head);
		return NULL;
	}
	head->count = 0;

	/* Use defaults if nothing specified */
	if(data_del){
		head->data_del = data_del;
	}else{
		head->data_del = default_data_del;
	}

	if(data_cmp){
		head->data_cmp = data_cmp;
	}else{
		head->data_cmp = default_data_cmp;
	}
	
	return head;
}

/*
  Will replace the value of the KV_PAIR object with the same key
  The node that is replaced is destroyed(free) in result
*/
void to_list_replace(L_HEAD *head, void *pair){
	KV_PAIR *p = NULL;
	KV_PAIR *pp = pair;
	L_NODE *n = head->node;
	while(n){
		p = n->data;
		if(head->data_cmp(p, pp->key)){
			/* we have match */
			head->data_del(p);
			n->data = pp;
			break;
		}
		n = n->next;
	}
}

void to_list_push(L_HEAD *head, void *data){
	L_NODE *n = head->node;
	
	if(!n->data){
		n->data = data;
	}else{
		while(n->next) n=n->next; /* fast forward to last node */
		
		L_NODE *nn = malloc(sizeof(L_NODE));
		memset(nn, 0, sizeof(L_NODE));
		nn->data = data;
		nn->next = NULL;
		n->next = nn;
	}
	head->count++;
	//fprintf(stderr, "Currently [%d] elements in head\n", head->count);
}

bool to_list_peek(L_HEAD *head, void *key){
	L_NODE *n = head->node;
	while(n && n->data){
		if(head->data_cmp(n->data, key)){
			return true;
		}
		n = n->next;
	}
	return false;
}

/* Returns number of elements with given key */
int to_list_get_count(L_HEAD *head, void *key){
	int count = 0;
	L_NODE *n = head->node;
		
	while(n){
		if(head->data_cmp(n->data, key)){
			count++;
		}
		n = n->next;
	}
	return count;
}

/* Returns node without removing it from list */
void * to_list_find(L_HEAD *head, void *key){
	L_NODE *n=head->node;
	while(n){
		if(head->data_cmp(n->data, key)){
			return n->data;
		}
		n = n->next;
	}
	return NULL;
}

/* Returns node and removes it from the list */
void * to_list_get(L_HEAD *head, void *key){
	L_NODE *prev = NULL;
	L_NODE *curr = head->node;
	while(curr){
		/* Keep the pointer to object we want to return */
		void *p = curr->data;
		if(head->data_cmp(curr->data, key)){
		        if(prev){
				prev->next = curr->next;
			}else{
				head->node = curr->next;
			}
			free(curr);
			head->count--;
			return p;
		}
		prev = curr;
		curr = curr->next;
	}
	return NULL;
}

/* Writes values of @head into one single @buffer 
   
   return number of bytes written to @buffer
   or -1 on error
 */
size_t to_list_2_buf(L_HEAD *head, char **buffer){
	size_t buflen = 0;

	if(!head && head->count < 1) return -1;
	
	for(L_NODE *n = head->node; n; n=n->next){
		KV_PAIR *p = n->data;
		buflen += p->vlen;
	}

	*buffer = malloc(buflen * sizeof(char));

	if(!(*buffer)) return -1;

	int here = 0;
	for(L_NODE *n = head->node; n; n=n->next){
		KV_PAIR *p = n->data;
		memcpy(*buffer + here, p->value, p->vlen);
		here += p->vlen;
	}

	return buflen;
}

void to_list_destroy(L_HEAD *head){
	L_NODE *n = head->node;
	L_NODE *nn = NULL;
	while (n){
		nn = n->next;
		head->data_del(n->data);
		free(n);
		n = nn;
	}
	head->node = NULL;
	free(head);
}

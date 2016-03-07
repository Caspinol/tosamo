#include "include/lists.h"


KV_PAIR *to_kvpair_create(char *key, size_t klen, char *value, size_t vlen){
	KV_PAIR *b = malloc(sizeof(KV_PAIR));
	if(!b){
		to_log_err("Cannot malloc() the KV_PAIR - exiting");
		return NULL;
	}
	
	b->value = calloc(vlen+1, sizeof(char));
	if(!b->value){
		to_log_err("Cannot malloc() the KV_PAIR->value - exiting");
		return NULL;
	}
	
	b->key = calloc(klen+1, sizeof(char));
	if(!b->key){
		to_log_err("Cannot create key");
		return NULL;
	}  
	strncpy(b->key, key, klen);
	strncpy(b->value, value, vlen);
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

/*
  New list instantiation with specified name
  
  @param name - name of the list (filename)

  @return h - pointer to a head of a list
  or NULL if unsuccessfull
 */
L_HEAD *to_list_create(){
	L_HEAD *h = calloc(1, sizeof(L_HEAD));
	if(!h) return NULL;
	
	h->node = calloc(1, sizeof(L_NODE));
	if(!h->node){
		free(h);
		return NULL;
	}
	h->count = 0;
	return h;
}

/*
  Will replace the value of the KV_PAIR object with the same key
  The node that is replaced is destroyed(free) in result
*/
void to_list_replace(L_HEAD *head, void *pair){
	KV_PAIR *p;
	KV_PAIR *pp = pair;
	
	for(L_NODE *n = head->node; n; n=n->next){
		p = n->data;
		if(!strncmp(p->key, pp->key, strlen(p->key))){
			/* we have match */
			to_kvpair_destroy(p);
			n->data = pp;
			break;
		}
	}
}

void to_list_push(L_HEAD *head, void *data){
	L_NODE *n = head->node;
	
	if(!n->data){
		n->data = data;
	}else{
		while(n->next) n=n->next; /* fast forward to last node */
		
		L_NODE *nn = calloc(1, sizeof(L_NODE));
		nn->data = data;
		nn->next = NULL;
		n->next = nn;
	}
	head->count++;
}

bool to_list_peek(L_HEAD *head, void *key){
	for(L_NODE *n = head->node; n; n = n->next){
		KV_PAIR *p = n->data;
		if(strncmp(p->key, key, strlen(key)) == 0){
			return true;
		}
	}
	return false;
}

/* Returns node without removing it from list */
void * to_list_find(L_HEAD *head, void *key){
	for(L_NODE *n=head->node; n; n=n->next){
		KV_PAIR *p = n->data;
		if(strncmp(p->key, key, strlen(key)) == 0){
			return p;
		}
	}
	return NULL;
}

/* Returns node and removes it from the list */
void * to_list_get(L_HEAD *head, void *key){
	L_NODE *prev = NULL;
	L_NODE *curr = head->node;
	while(curr){
		/* Keep the pointer to object we want to return */
		KV_PAIR *p = curr->data;
		if(strncmp(p->key, key, strlen(key)) == 0){
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

void to_list_destroy(L_HEAD *head){
	L_NODE *n = head->node;
	L_NODE *nn;
	while (n){
		nn = n->next;
		to_kvpair_destroy(n->data);
		free(n);
		n = nn;
	}
	head->node = NULL;
	free(head);
}

/* $Id$ */

#include "n_hash_int.h"

void *n_hash_get(const tn_hash *ht, const char *key)
{
    unsigned val = ht->hash_fn(key) % ht->size;
    struct hash_bucket *ptr;

    if (NULL == ht->table[val])
	return NULL;

    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
	if (0 == strcmp(key, ptr->key))
	    return ptr->data;
    }

    return NULL;
}

/* $Id$ */

#include "n_hash_int.h"

int n_hash_exists(const tn_hash *ht, const char *key) 
{
    unsigned val = ht->hash_fn(key) % ht->size;
    struct hash_bucket *ptr;

    if (NULL == ht->table[val])
	return 0;

    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
	if (0 == strcmp(key, ptr->key))
	    return 1;
    }

    return 0;
}

/* $Id$ */

#include "n_hash_int.h"

int n_hash_map(const tn_hash *ht, void (*map_fn) (const char *, void *))
{
    register size_t i, n = 0;
    register struct hash_bucket *tmp;

    for (i = 0; i < ht->size; i++) {
        if (ht->table[i] == NULL)
            continue;
        
        for (tmp = ht->table[i]; tmp != NULL; tmp = tmp->next) {
            map_fn(tmp->key, tmp->data);
            n++;
        }
    }

    return n;
}

/* $Id$ */

#include "n_hash_int.h"
#include "narray.h"

tn_array *n_hash_keys(const tn_hash *ht)
{
    register size_t i;
    register struct hash_bucket *tmp;
    tn_array *keys; 

    keys = n_array_new(ht->items, NULL, (tn_fn_cmp)strcmp);
    
    for (i = 0; i < ht->size; i++) {
        if (ht->table[i] == NULL)
            continue;
        
        for (tmp = ht->table[i]; tmp != NULL; tmp = tmp->next)
            n_array_push(keys, tmp->key);
    }

    return keys;
}

/* $Id$ */

#include "n_hash_int.h"

static inline
int remove_bucket_ll(struct hash_bucket **tbl, const char *key,
                     unsigned val, void **data)
{
    struct hash_bucket *ptr, *last = NULL;

    n_assert (tbl[val] != NULL);

    for (last = NULL, ptr = tbl[val]; ptr != NULL;
         last = ptr, ptr = ptr->next) {

        if (strcmp(key, ptr->key) == 0) {
            *data = ptr->data;
            
            if (last != NULL)
                last->next = ptr->next;
            else 
                tbl[val] = ptr->next;
            
            free(ptr);
            return 1;
        }
    }

    return 0;
}

void *n_hash_remove(tn_hash *ht, const char *key)
{
    struct hash_bucket **tbl;
    void  *ptr = NULL;
    unsigned val;
    int klen;

    val = n_hash_dohash(ht, key, &klen);
    tbl = ht->table;
    
    if (tbl[val] != NULL)
        if (remove_bucket_ll(tbl, key, val, &ptr)) {
            ht->items--;
            return ptr;
        }
    
    val += n_hash_nextslotv(key, klen);
    if (val < ht->size && tbl[val])
        if (remove_bucket_ll(tbl, key, val, &ptr)) {
            ht->items--;
            return ptr;
        }
    
    return NULL;
}


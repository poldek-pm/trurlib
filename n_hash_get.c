/* $Id$ */

#include "n_hash_int.h"

#if 0
void *n_hash_get_ll(const tn_hash *ht, const char *key)
{
    unsigned val = n_hash_dohash(ht, key, NULL);
    struct hash_bucket *ptr;

    if (ht->table[val] == NULL)
        return NULL;

    for (ptr = ht->table[val]; ptr != NULL; ptr = ptr->next) {
        if (strcmp(key, ptr->key) == 0)
            return ptr->data;
    }

    return NULL;
}

int n_hash_exists_ll(const tn_hash *ht, const char *key) 
{
    unsigned val = n_hash_dohash(ht, key, NULL);
    struct hash_bucket *ptr;

    if (NULL == ht->table[val])
        return 0;

    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
        if (0 == strcmp(key, ptr->key))
            return 1;
    }
    
    return 0;
}
#endif

static inline
struct hash_bucket *get_bucket_ll(struct hash_bucket **tbl,
                                         const char *key, unsigned val)
{
    struct hash_bucket *ptr;

    n_assert (tbl[val] != NULL);
    
    for (ptr = tbl[val]; ptr != NULL; ptr = ptr->next) {
        if (strcmp(key, ptr->key) == 0)
            return ptr;
    }
    return NULL;
}


struct hash_bucket *n_hash_get_bucket(const tn_hash *ht,
                                      const char *key, int klen, unsigned val)
{
    struct hash_bucket **tbl, *ptr = NULL;
    tbl = ht->table;
    
    if (tbl[val] != NULL)
        ptr = get_bucket_ll(tbl, key, val);

    if (ptr == NULL) {
        val += n_hash_nextslotv(key, klen);
        if (val < ht->size && tbl[val])
            ptr = get_bucket_ll(tbl, key, val);
    }

    return ptr;
}

void *n_hash_get(const tn_hash *ht, const char *key) 
{
    struct hash_bucket *ptr;
    unsigned val;
    int klen;
    
    val = n_hash_dohash(ht, key, &klen);
    ptr = n_hash_get_bucket(ht, key, klen, val);
    return ptr ? ptr->data : NULL;
}

int n_hash_exists(const tn_hash *ht, const char *key) 
{
    unsigned val;
    int klen;
    
    val = n_hash_dohash(ht, key, &klen);
    return n_hash_get_bucket(ht, key, klen, val) != NULL;
}

    

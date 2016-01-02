#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

static inline
struct hash_bucket *get_bucket_ll(struct hash_bucket **tbl,
                                         const char *key, unsigned khash)
{
    struct hash_bucket *ptr;

    n_assert (tbl[khash] != NULL);
    
    for (ptr = tbl[khash]; ptr != NULL; ptr = ptr->next) {
        if (strcmp(key, ptr->key) == 0)
            return ptr;
    }
    return NULL;
}


struct hash_bucket *n_hash_get_bucket(const tn_hash *ht,
                                      const char *key, int klen, unsigned khash)
{
    struct hash_bucket **tbl, *ptr = NULL;
    tbl = ht->table;
    
    if (tbl[khash] != NULL)
        ptr = get_bucket_ll(tbl, key, khash);

    if (ptr == NULL) {
        khash += n_hash_nextslotv(key, klen);
        if (khash < ht->size && tbl[khash])
            ptr = get_bucket_ll(tbl, key, khash);
    }

    return ptr;
}

void *n_hash_get_ex(const tn_hash *ht,
                    const char *key, int *klen, unsigned *khash)
{
    struct hash_bucket *ptr;
    
    *khash = n_hash_dohash(ht, key, klen);
    ptr = n_hash_get_bucket(ht, key, *klen, *khash);
    return ptr ? ptr->data : NULL;
}


void *n_hash_get(const tn_hash *ht, const char *key) 
{
    unsigned khash;
    int klen;

    return n_hash_get_ex(ht, key, &klen, &khash);
}

int n_hash_exists_ex(const tn_hash *ht, const char *key, int *klen,
                     unsigned *khash)
{
    *khash = n_hash_dohash(ht, key, klen);
    return n_hash_get_bucket(ht, key, *klen, *khash) != NULL;
}

int n_hash_exists(const tn_hash *ht, const char *key) 
{
    int klen;
    unsigned khash;
    
    return n_hash_exists_ex(ht, key, &klen, &khash);
}

    

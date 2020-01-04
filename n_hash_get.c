#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

void n_hash_it_init(tn_hash_it *hi, tn_hash *ht)
{
    hi->ht = ht;
    hi->pos = 0;
}

void *n_hash_it_get(tn_hash_it *hi, const char **key) {
    struct hash_bucket **tbl = hi->ht->table;
    size_t i = hi->pos;

    while (tbl[i] == NULL && i < hi->ht->size)
        i++;

    if (i >= hi->ht->size)
        return NULL;

    struct hash_bucket *ptr = tbl[i];

    hi->pos = i + 1;
    if (key)
        *key = ptr->key;

    return ptr->data;
}


static inline
struct hash_bucket *get_bucket_ll(struct hash_bucket **tbl,
                                  const char *key, unsigned klen, unsigned khash)
{
    struct hash_bucket *ptr;

    n_assert(tbl[khash] != NULL);

    for (ptr = tbl[khash]; ptr != NULL; ptr = ptr->next) {
        if (klen == ptr->klen && strcmp(key, ptr->key) == 0)
            return ptr;
    }
    return NULL;
}


struct hash_bucket *n_hash_get_bucket(const tn_hash *ht,
                                      const char *key, int klen, unsigned khash)
{
    struct hash_bucket **tbl, *ptr = NULL;
    tbl = ht->table;

    n_assert(klen > 0);
    n_assert(klen < UINT16_MAX);

    if (tbl[khash] != NULL)
        ptr = get_bucket_ll(tbl, key, klen, khash);

    if (ptr == NULL) {
        uint32_t shift = n_hash_nextslotv(key, klen);
        uint32_t nexthash = khash + shift;

        if (nexthash < ht->size && tbl[nexthash])
            ptr = get_bucket_ll(tbl, key, klen, nexthash);

        if (ptr == NULL && khash > shift) {
            nexthash = khash - shift;
            if (tbl[nexthash])
                ptr = get_bucket_ll(tbl, key, klen, nexthash);
        }
    }
    return ptr;
}

void *n_hash_get_ex(const tn_hash *ht,
                    const char *key, int *klen, unsigned *khash)
{
    struct hash_bucket *ptr;

    *klen = strlen(key);
    *khash = n_hash_compute_hash(ht, key, *klen);

    ptr = n_hash_get_bucket(ht, key, *klen, *khash);
    return ptr ? ptr->data : NULL;
}


void *n_hash_get(const tn_hash *ht, const char *key)
{
    int klen;
    unsigned khash;

    return n_hash_get_ex(ht, key, &klen, &khash);
}

int n_hash_exists_ex(const tn_hash *ht, const char *key, int *klen,
                     unsigned *khash)
{
    *khash = n_hash_compute_hash_len(ht, key, klen);
    return n_hash_get_bucket(ht, key, *klen, *khash) != NULL;
}

int n_hash_exists(const tn_hash *ht, const char *key)
{
    int klen = strlen(key);
    unsigned khash;

    return n_hash_exists_ex(ht, key, &klen, &khash);
}

/* h API */
int n_hash_hexists(const tn_hash *ht, const char *key, int klen, unsigned khash)
{
    return n_hash_get_bucket(ht, key, klen, khash) != NULL;
}

void *n_hash_hget(const tn_hash *ht, const char *key, int klen, unsigned khash)
{
    struct hash_bucket *ptr;

    ptr = n_hash_get_bucket(ht, key, klen, khash);
    return ptr ? ptr->data : NULL;
}

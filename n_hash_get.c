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


struct hash_bucket *n_hash_get_bucket(const tn_hash *ht,
                                      const char *key, int klen, uint32_t khash,
                                      uint32_t *bindex)
{
    struct hash_bucket **table = ht->table;

    //n_assert(klen > 0);
    //n_assert(klen < UINT16_MAX);

    uint32_t size = ht->size;
    uint32_t index = n_hash_index(size, khash);
    uint32_t i = index;
    uint32_t np = 0;

    struct hash_bucket *e = table[i];
    while (e != NULL) {
        uint32_t diff = n_hash_bucket_psldiff(ht, e, i);
        if (np > diff) {
            break;
        }

        if (i != index && e->psl == 0)
            break;

        np++;
        if (khash == e->hash && klen == e->klen &&
            *key == *e->key && memcmp(key, e->key, klen) == 0) {
            if (bindex)
                *bindex = i;
            return e;
        }

        i = n_hash_index(size, i + 1);
        e = table[i];
        n_assert(i != index);
    }

    return NULL;
}

void *n_hash_get_ex(const tn_hash *ht,
                    const char *key, int *klen, unsigned *khash)
{
    struct hash_bucket *ptr;

    *klen = strlen(key);
    *khash = n_hash_compute_raw_hash(key, *klen);

    ptr = n_hash_get_bucket(ht, key, *klen, *khash, NULL);
    return ptr ? ptr->data : NULL;
}


void *n_hash_get(const tn_hash *ht, const char *key)
{
    int klen;
    unsigned khash;

    return n_hash_get_ex(ht, key, &klen, &khash);
}

int n_hash_exists_ex(const tn_hash *ht, const char *key, int klen,
                     unsigned *khash)
{
    *khash = n_hash_compute_hash(ht, key, klen);
    return n_hash_get_bucket(ht, key, klen, *khash, NULL) != NULL;
}

int n_hash_exists(const tn_hash *ht, const char *key)
{
    int klen = strlen(key);
    unsigned khash;

    return n_hash_exists_ex(ht, key, klen, &khash);
}

/* h API */
int n_hash_hexists(const tn_hash *ht, const char *key, int klen, unsigned khash)
{
    return n_hash_get_bucket(ht, key, klen, khash, NULL) != NULL;
}

void *n_hash_hget(const tn_hash *ht, const char *key, int klen, unsigned khash)
{
    struct hash_bucket *ptr;

    ptr = n_hash_get_bucket(ht, key, klen, khash, NULL);
    return ptr ? ptr->data : NULL;
}

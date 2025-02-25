#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

void *n_hash_remove(tn_hash *ht, const char *key)
{
    int klen = strlen(key);
    uint32_t khash = n_hash_compute_hash(ht, key, klen);

    uint32_t eindex = 0;
    struct hash_bucket *e = n_hash_get_bucket(ht, key, klen, khash, &eindex);

    if (e == NULL)
        return NULL;

    uint32_t size = ht->size;
    uint32_t iprev = eindex;
    uint32_t i = n_hash_index(size, eindex + 1);

    struct hash_bucket **table = ht->table;
    struct hash_bucket *b = table[i];

    while (b && b->psl > 0) {
        //n_assert(n_hash_bucket_psldiff(ht, b, i) == b->psl);

        table[iprev] = b;
        b->psl -= 1;

        iprev = i;
        i = n_hash_index(size, i + 1);
        b = table[i];
    }
    table[iprev] = NULL;

    void *data = e->data;
    if (!ht->na)
        free(e);

    ht->items -= 1;
    return data;
}

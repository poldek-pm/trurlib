#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

int n_hash_map(const tn_hash *ht, void (*map_fn) (const char *, void *))
{
    register size_t i, n = 0;
    register struct hash_bucket *tmp;

    for (i = 0; i < ht->size; i++) {
        if (ht->table[i] == NULL)
            continue;

        tmp = ht->table[i];
        map_fn(tmp->key, tmp->data);
        n++;
    }

    return n;
}

int n_hash_dump(const tn_hash *ht)
{
    uint32_t i, n = 0;

    printf("DUMP %p size=%d\n", ht, ht->items);
    for (i = 0; i < ht->size; i++) {
        struct hash_bucket *e = ht->table[i];

        if (e) {
            unsigned d = n_hash_bucket_psldiff(ht, e, i);
            printf("[%u] %u. psl %d (%d)%s: %s => %p\n", i, n, e->psl, d,
                   d != e->psl ? " DIFFER" : "", e->key, e->data);
            n++;
        } else {
            printf("[%u] empty\n", i);
        }


    }

    return n;
}

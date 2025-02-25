#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"


void n_hash_free(tn_hash *ht)
{
    size_t i;

    if (ht->_refcnt > 0) {
        ht->_refcnt--;
        return;
    }

#if ENABLE_TRACE
    n_hash_stats(ht);
#endif

    for (i = 0; i < ht->size; i++) {
        struct hash_bucket *e = ht->table[i];
        if (e == NULL)
            continue;

        if (ht->free_fn != NULL && e->data != NULL) {
            ht->free_fn(e->data);
            e->data = NULL;

            if (ht->na == NULL) {
                free(e);
                ht->table[i] = NULL;
            }
        }
    }

    free(ht->table);

    ht->table = NULL;
    ht->size = 0;
    ht->items = 0;

    if (ht->na == NULL)
        free(ht);
    else
        n_alloc_free(ht->na);
}

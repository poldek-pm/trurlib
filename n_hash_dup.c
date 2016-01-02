#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

tn_hash *n_hash_dup(const tn_hash *ht, tn_fn_dup dup_fn)
{
    register size_t  i, n = 0;
    register struct  hash_bucket *tmp;
    tn_hash          *h;

    h = n_hash_new(ht->size, NULL);
    h->size = ht->size;
    h->items = 0;
    h->free_fn = ht->free_fn;
    h->hash_fn = ht->hash_fn;
    h->flags = h->flags;
    h->flags &= ~(TN_HASH_NOCPKEY);
    for (i = 0; i < ht->size; i++) {
        if (ht->table[i] == NULL)
            continue;
        
        for (tmp = ht->table[i]; tmp != NULL; tmp = tmp->next) {
            void *data = tmp->data;
            if (data && dup_fn)
                data = dup_fn(data);
            
            n_hash_insert(h, tmp->key, data);
            n++;
        }
    }

    return h;
}

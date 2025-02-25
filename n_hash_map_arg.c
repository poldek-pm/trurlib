#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

int n_hash_map_arg(const tn_hash *ht,
		   void (*map_fn) (const char *, void *, void *),
		   void *arg)
{
    register size_t i, n = 0;
    register struct hash_bucket *tmp;

    for (i = 0; i < ht->size; i++) {
        tmp = ht->table[i];
        map_fn(tmp->key, tmp->data, arg);
        n++;
    }

    return n;
}

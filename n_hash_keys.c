#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"
#include "narray.h"
#include "nmalloc.h"

tn_array *n_hash_keys_ext(const tn_hash *ht, int cp)
{
    register size_t i;
    register struct hash_bucket *tmp;
    tn_array *keys;
    tn_fn_free fn = NULL;

    if (cp)
        fn = free;

    keys = n_array_new(ht->items, fn, (tn_fn_cmp)strcmp);

    for (i = 0; i < ht->size; i++) {
        if (ht->table[i] == NULL)
            continue;

        tmp = ht->table[i];
        n_array_push(keys, cp ? n_strdup(tmp->key) : tmp->key);
    }

    return keys;
}

#undef n_hash_keys
tn_array *n_hash_keys(const tn_hash *ht)
{
    return n_hash_keys_ext(ht, 0);
}

tn_array *n_hash_values(const tn_hash *ht)
{
    register size_t i;
    register struct hash_bucket *tmp;
    tn_array *values;

    values = n_array_new(ht->items, NULL, NULL);

    for (i = 0; i < ht->size; i++) {
        if (ht->table[i] == NULL)
            continue;

        tmp = ht->table[i];
        n_array_push(values, tmp->data);
    }

    return values;
}

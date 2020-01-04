#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

static inline
int remove_bucket_ll(struct hash_bucket **tbl, const char *key,
                     unsigned val, void **data)
{
    struct hash_bucket *ptr, *last = NULL;

    n_assert (tbl[val] != NULL);

    for (last = NULL, ptr = tbl[val]; ptr != NULL;
         last = ptr, ptr = ptr->next) {

        if (strcmp(key, ptr->key) == 0) {
            *data = ptr->data;

            if (last != NULL)
                last->next = ptr->next;
            else
                tbl[val] = ptr->next;

            free(ptr);
            return 1;
        }
    }

    return 0;
}

void *n_hash_remove(tn_hash *ht, const char *key)
{
    struct hash_bucket **tbl;
    void  *ptr = NULL;
    uint32_t khash;
    int klen = strlen(key);

    khash = n_hash_compute_hash(ht, key, klen);
    tbl = ht->table;

    if (tbl[khash] != NULL) {
        if (remove_bucket_ll(tbl, key, khash, &ptr)) {
            ht->items--;
            return ptr;
        }
    }
    uint32_t shift = n_hash_nextslotv(key, klen);

    khash += shift;
    if (khash < ht->size && tbl[khash] != NULL) {
        if (remove_bucket_ll(tbl, key, khash, &ptr)) {
            ht->items--;
            return ptr;
        }
    }

    khash -= shift;
    if (khash > shift) {
        khash -= shift;
        if (tbl[khash] != NULL) {
            if (remove_bucket_ll(tbl, key, khash, &ptr)) {
                ht->items--;
                return ptr;
            }
        }
    }

    return NULL;
}

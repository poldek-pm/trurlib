#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

void n_hash_clean(tn_hash *ht)
{
    size_t i;
    for (i = 0; i < ht->size; i++) {
	while (ht->table[i] != NULL) {
	    void *d = n_hash_remove(ht, ht->table[i]->key);
	    if (ht->free_fn != NULL && d != NULL)
		ht->free_fn(d);
            
            ht->table[i] = NULL;
	}
    }

    ht->items = 0;
}

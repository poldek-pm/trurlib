/* $Id$ */

#include "n_hash_int.h"

void n_hash_free(tn_hash *ht)
{
    size_t i;
    
    if (ht->_refcnt > 0) {
        ht->_refcnt--;
        return;
    }
    
    
    for (i = 0; i < ht->size; i++) {
        while (ht->table[i] != NULL) {
            void *d = n_hash_remove(ht, ht->table[i]->key);
            if (ht->free_fn != NULL && d != NULL)
                ht->free_fn(d);
        }
    }
    free(ht->table);
    
    ht->table = NULL;
    ht->size = 0;
    ht->items = 0;

    free(ht);
}

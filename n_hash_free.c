/* $Id$ */

#include "n_hash_int.h"

static
void free_slot(struct hash_bucket *node, tn_hash *ht)
{
    register struct hash_bucket *next_node;

    for (; node != NULL; node = next_node) {
        next_node = node->next;

        if (ht->free_fn != NULL && node->data != NULL) {
            DBGF("free %s %p\n", node->key, node->data);
            ht->free_fn(node->data);
        }
        free(node);
    }
}


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
        if (ht->table[i])
            free_slot(ht->table[i], ht);
    }

    free(ht->table);
    
    ht->table = NULL;
    ht->size = 0;
    ht->items = 0;

    free(ht);
}

/* $Id$ */

#include "n_hash_int.h"

#include <stdio.h>
int n_hash_stats(const tn_hash *ht)
{
    register size_t i, n = 0;
    register struct hash_bucket *tmp;
    int ncolls = 0;
    int nempts = 0;
    int maxdeep = 0;

    
    for (i = 0; i < ht->size; i++) {
        int deep;
        
	if (ht->table[i] == NULL) {
            nempts++;
            continue;
        }
        
        deep = 1;
        for (tmp = ht->table[i]->next; tmp != NULL; tmp = tmp->next) 
            deep++;
        
        if (deep > 1)
            ncolls++;
        
        if (deep > maxdeep)
            maxdeep = deep;
    }

    printf("%p %zd items, %d emptys, %d collisions, maxdeep %d\n", ht, 
           ht->items, nempts, ncolls, maxdeep);
    return n;
}

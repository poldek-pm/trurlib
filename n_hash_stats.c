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
    int deepsum = 0;
    int empty_len_sum = 0;
    int empty_len_n = 0;
    int empty_len = 0;
    
    for (i = 0; i < ht->size; i++) {
        int deep;
        
        if (ht->table[i] == NULL) {
            nempts++;
            empty_len++;
            continue;
        }
        if (empty_len > 6) {
            empty_len_n++;
            empty_len_sum += empty_len;
            empty_len = 0;
        }
        
        
        deep = 1;
        for (tmp = ht->table[i]->next; tmp != NULL; tmp = tmp->next) 
            deep++;
        
        if (deep > 1) {
            ncolls++;
            deepsum += deep;
        }
        
        if (deep > maxdeep)
            maxdeep = deep;
    }

    printf("ht(%p): %d slots (%d empty, avg empty len %d), %d items, "
           "%d collisions (avgdeep %.2lf, maxdep %d)\n",
           ht, ht->size, nempts, empty_len_sum ? empty_len_sum / empty_len_n : 0,
           ht->items, ncolls,
           ncolls > 0 && deepsum > 0 ? deepsum/(ncolls * 1.0) : 0,
           maxdeep);
//    n_assert(0);
    return n;
}

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

#include <stdio.h>
int n_hash_stats(const tn_hash *ht)
{
    register size_t i, n = 0;
    register struct hash_bucket *tmp;
    int ncolls = 0;
    int maxdeep = 0;
    int deepsum = 0;
    int empty_len_sum = 0;
    int empty_len_n = 0;
    int empty_len = 0;

    for (i = 0; i < ht->size; i++) {
        int deep;

        if (ht->table[i] == NULL) {
            empty_len++;
            continue;
        }
        if (empty_len > 0) {
            empty_len_n++;
            empty_len_sum += empty_len;
        }
        empty_len = 0;


        deep = 1;
        //if (ht->table[i]->next)
        //    printf("coll %s, ", ht->table[i]->key);

        for (tmp = ht->table[i]->next; tmp != NULL; tmp = tmp->next) {
            //printf("%s, ", tmp->key);
            deep++;
        }
        //if (ht->table[i]->next)
        //    printf("\n");

        if (deep > 1) {
            ncolls++;
            deepsum += deep;
        }

        if (deep > maxdeep)
            maxdeep = deep;
    }

    if (empty_len > 0) {        /* a run at the end of the table */
        empty_len_n++;
        empty_len_sum += empty_len;
    }

    printf("ht(%p): %u keys, %u slots (%.2lf load, avg empty len %.lf), "
           "%d collisions (%.2lf%%, avgdeep %.2lf, maxdeep %d)\n",
           ht, ht->items, ht->size, (100.0 * ht->items) / ht->size, empty_len_sum ? empty_len_sum / (1.0 * empty_len_n) : 0,
           ncolls, 100 * (ncolls/ (ht->items * 1.0)),
           ncolls > 0 && deepsum > 0 ? deepsum/(ncolls * 1.0) : 0,
           maxdeep);
//    n_assert(0);
    return n;
}

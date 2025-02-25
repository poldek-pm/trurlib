#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_hash_int.h"

#include <stdio.h>
int n_hash_stats(const tn_hash *ht)
{
    register size_t i, n = 0;
    int nempts = 0;
    int pslsum = 0;
    int empty_len_sum = 0;
    int empty_blocks = 0;
    int empty_len = 0;
    int maxpsl = 0;

    int nonempty_len = 0;
    int nonempty_blocks = 0;

    for (i = 0; i < ht->size; i++) {
        if (i > 0) {
            if (ht->table[i-1] != NULL && ht->table[i] == NULL) {
                nonempty_blocks++;
                nonempty_len = 0;
            } else if (ht->table[i-1] == NULL && ht->table[i] != NULL) {
                empty_blocks++;
                empty_len_sum += empty_len;
                empty_len = 0;
            }
        }

        if (ht->table[i] == NULL) {
            nempts++;
            empty_len++;
            continue;
        } else {
            nonempty_len++;
        }

        pslsum += ht->table[i]->psl;
        if (ht->table[i]->psl > maxpsl)
            maxpsl = ht->table[i]->psl;
    }

    printf("ht(%p): %u slots, %u items (%.2f%% empty, avg empty len %.lf), avg psl %.1lf, max psl %d)\n",
           ht,
           ht->size,
           ht->items,
           (100.0 * nempts)/ht->size,
           empty_len_sum ? empty_len_sum / (1.0 * empty_blocks) : 0,
           pslsum > 0 ? pslsum/(ht->items * 1.0) : 0,
           maxpsl);
    return n;
}

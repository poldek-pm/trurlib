/* $Id$ */

#include "n_hash_int.h"

int n_hash_ctl(tn_hash *ht, unsigned int flags)
{
    if (ht->items > 0) {
        trurl_die("n_hash_ctl: hash table not empty");
        return 0;
    }
    ht->flags |= flags;
    return 1;
}

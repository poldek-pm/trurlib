#include <string.h>
#include <stdlib.h>

#include "trurl_internal.h"

struct dentry {
    void    *data;
    char    *key;
    char    _buf[];
};

struct dentry *n_oash__get_insert(tn_oash *ht, const char *key, size_t len);

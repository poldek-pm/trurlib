#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_list_int.h"

int n_list_size(const tn_list *l)
{
    return l->items;
}

/* $Id$ */

#include "n_list_int.h"

tn_list *n_list_new(unsigned int flags, t_fn_free freef, t_fn_cmp cmpf)
{
    tn_list *l;

    if ((l = n_malloc(sizeof(*l))) == NULL)
        return NULL;

    l->_refcnt = 0;
    l->flags = flags;
    l->items = 0;

    l->head = l->tail = NULL;

    l->free_fn = freef;
    if (cmpf != NULL)
        l->cmp_fn = cmpf;
    else 
        l->cmp_fn = trurl_default_cmpf;

    return l;
}

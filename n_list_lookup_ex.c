#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_list_int.h"

void *n_list_lookup_ex(const tn_list *l, const void *data, t_fn_cmp cmpf)
{
    register struct list_node *node;


    if (cmpf == NULL)
	cmpf = l->cmp_fn;

    if (cmpf == NULL) {
	trurl_die("n_list_lookup: cmpf function is NULL\n");
	return NULL;
    }
    
    for (node = l->head; node != NULL; node = node->next) {
	if (cmpf(node->data, data) == 0)
	    return node->data;
    }

    return NULL;
}

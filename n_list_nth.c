#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_list_int.h"

void *n_list_nth(const tn_list *l, int nth)
{
    register struct list_node *node;
    register int n = 0;

    for (node = l->head; node != NULL; node = node->next) {
	if (n == nth)
	    return node->data;
	n++;
    }

    return NULL;
}

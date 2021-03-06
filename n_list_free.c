#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_list_int.h"

void n_list_free(tn_list *l)
{
    register struct list_node *node, *next_node;

    if (l->_refcnt > 0) {
        l->_refcnt--;
        return;
    }

    for (node = l->head; node != NULL; node = next_node) {
        next_node = node->next;

        if (l->free_fn != NULL && node->data != NULL)
            l->free_fn(node->data);

        free(node);
    }

    free(l);
}

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "n_list_int.h"

int n_list_remove_ex(tn_list *l, const void *data, t_fn_cmp cmpf)
{
    register struct list_node *node, *prev_node;
    int removed = 0;

    if (l->head == NULL)
	return 0;

    if (cmpf == NULL)
	cmpf = l->cmp_fn;


    n_assert(cmpf != NULL);
    if (cmpf == NULL) {
	trurl_die("n_list_remove_ex: compare function is NULL");
	return -1;
    }
    
    node = l->head;

    /* remove from the beginning */
    while (node != NULL && cmpf(node->data, data) == 0) {
	if (l->free_fn != NULL)
	    l->free_fn(node->data);

	l->head = l->head->next;
	free(node);
	node = l->head;
	l->items--;
	removed++;
        if (l->items > 1) 
            n_assert(l->head->next);
    }

    if (l->head == NULL) {
	n_assert(l->items == 0);
	return removed;

    } else if (removed > 0 && (l->flags & TN_LIST_UNIQ)) {
	return removed;

    } else {
	for (prev_node = l->head, node = l->head->next; node != NULL;
	     prev_node = node, node = node->next) {

	    if (cmpf(node->data, data) == 0) {
		if (node->data != NULL && l->free_fn != NULL)
		    l->free_fn(node->data);

		n_assert(prev_node != NULL);
		prev_node->next = node->next;
                if (node == l->tail)
                    l->tail = prev_node;
		free(node);

		n_assert(l->items > 0);

		l->items--;
		removed++;

		if (l->flags & TN_LIST_UNIQ) {
		    break;
                    
		} else {	/* duplicates allowed */
		    n_assert(prev_node != NULL);
		    node = prev_node;
		}
	    }
	}
    }

    return removed;
}

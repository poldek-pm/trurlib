/* $Id$ */

#include "n_list_int.h"

void *n_list_remove_nth(tn_list *l, int nth)
{
    void *data;


    if (l->head == NULL) {
	trurl_die("n_list_remove_nth: remove from empty list\n");
	return NULL;
    }
    
    if (nth < 0 || l->items <= nth) {
	trurl_die("n_list_remove_nth: index(%d) out of bounds(%d)\n", nth,
		  l->items);

	return NULL;
    }
    
    if (nth == 0) {
	data = n_list_shift(l);

    } else if (nth == l->items - 1) {
	data = n_list_pop(l);

    } else {

	register struct list_node *node, *prev_node;
	register size_t n = 0;

	for (prev_node = l->head, node = l->head->next; node != NULL;
	     prev_node = node, node = node->next) {

	    n++;

	    if (n == (size_t) nth)
		break;
	}

	n_assert(node != NULL);

	l->items--;

	prev_node->next = node->next;
	data = node->data;
	free(node);
    }

    return data;
}

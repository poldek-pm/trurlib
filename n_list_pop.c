/* $Id$ */

#include "n_list_int.h"

void *n_list_pop(tn_list *l)
{
    void *data;

    if (l->head == NULL) {
	trurl_die("n_list_pop: pop from empty list\n");
	return NULL;
    }
    
    if (l->head == l->tail) {
	data = l->head->data;
	free(l->head);
	l->head = l->tail = NULL;

    } else {
	register struct list_node *node, *tail_node;

	tail_node = l->tail;

	for (node = l->head; node->next != tail_node; node = node->next)
            ;

	n_assert(node != tail_node);

	data = tail_node->data;
	free(tail_node);

	node->next = NULL;
	l->tail = node;
    }

    l->items--;
    return data;
}

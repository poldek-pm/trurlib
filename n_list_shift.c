/* $Id$ */

#include "n_list_int.h"

void *n_list_shift(tn_list *l)
{
    register struct list_node *node;
    void *data;


    if (l->head == NULL) {
	trurl_die("n_list_shift: shift from empty list\n");
	return NULL;
    }
    
    node = l->head;
    l->head = l->head->next;

    if (l->head == NULL)	/* if(head == tail) */
	l->tail = NULL;

    data = node->data;
    free(node);


    l->items--;

    return data;
}

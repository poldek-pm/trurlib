/* $Id$ */

#include "n_list_int.h"

tn_list *n_list_push(tn_list *l, void *data)
{
    struct list_node *node;

    if (l->flags & TN_LIST_UNIQ) {
	if (n_list_contains_ex(l, data, NULL))
	    return NULL;
    }
    
    if ((node = malloc(sizeof(*node))) == NULL)
	return NULL;


    node->data = data;
    node->next = NULL;

    if (l->head == NULL) {	/* empty list */
	l->head = l->tail = node;

    } else {
	l->tail->next = node;
	l->tail = node;
    }

    l->items++;

    return l;
}

/* $Id$ */

#include "n_list_int.h"

tn_list *n_list_unshift(tn_list *l, void *data)
{
    struct list_node *node;

    if (l->flags & TN_LIST_UNIQ) {
	if (n_list_contains_ex(l, data, NULL))
	    return NULL;
    }
    
    if ((node = n_malloc(sizeof(*node))) == NULL) {
	return NULL;
    }
    
    node->data = data;
    node->next = l->head;
    l->head = node;

    l->items++;

    return l;
}

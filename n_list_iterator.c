/* $Id$ */

#include "n_list_int.h"

void n_list_iterator_start(const tn_list *l, tn_list_iterator *li)
{
    li->node = l->head;
}


void *n_list_iterator_get(tn_list_iterator *li)
{
    void *data;
    struct list_node *node;
    
    if (li == NULL)
        return NULL;
    
    node = li->node;
    
    if (node == NULL)
	return NULL;

    data = node->data;
    node = node->next;
    li->node = node;
    return data;
}

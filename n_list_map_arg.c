/* $Id$ */

#include "n_list_int.h"

void n_list_map_arg(const tn_list *l, void (*map_fn)(void *,void *), void *arg)
{
    register struct list_node *node;

    if (map_fn == NULL) {
	trurl_die("n_list_map_arg: map_fn function is NULL\n");
	return;
    }
    
    for (node = l->head; node != NULL; node = node->next) 
	map_fn(node->data, arg);
}

/* 
   TRURLib
   Copyright (C) 1999, 2000 Pawel A. Gajda (mis@k2.net.pl)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

/*
   $Id$
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_N_ASSERT
# include "nassert.h"
#else
#include <assert.h>
# define n_assert(expr) assert(expr)
#endif

#ifdef USE_XMALLOCS
# include "xmalloc.h"
#endif

#include "trurl_internal.h"
#include "nlist.h"

#ifndef MODULES
# define MODULE_n_list_new
# define MODULE_n_list_free
# define MODULE_n_list_push
# define MODULE_n_list_pop
# define MODULE_n_list_shift
# define MODULE_n_list_unshift
# define MODULE_n_list_remove_nth
# define MODULE_n_list_remove_ex
# define MODULE_n_list_lookup_ex
# define MODULE_n_list_contains_ex
# define MODULE_n_list_nth
# define MODULE_n_list_iterator
# define MODULE_n_list_size
# define MODULE_n_list_map_arg
#endif

struct list_node {
    void *data;
    struct list_node *next;
};

struct trurl_list {
    unsigned int flags;
    int items;

    struct list_node *head;
    struct list_node *tail;

    t_fn_free free_fn;
    t_fn_cmp cmp_fn;
};

#ifdef MODULE_n_list_new
tn_list *n_list_new(unsigned int flags, t_fn_free freef, t_fn_cmp cmpf)
{
    tn_list *l;

    if ((l = malloc(sizeof(*l))) == NULL)
	return NULL;

    l->flags = flags;
    l->items = 0;

    l->head = l->tail = NULL;

    l->free_fn = freef;
    if (cmpf != NULL)
        l->cmp_fn = cmpf;
    else 
        l->cmp_fn = trurl_default_cmpf;

    return l;
}
#endif


#ifdef MODULE_n_list_free
void n_list_free(tn_list *l)
{
    register struct list_node *node, *next_node;

    for (node = l->head; node != NULL; node = next_node) {
	next_node = node->next;

	if (l->free_fn != NULL && node->data != NULL)
	    l->free_fn(node->data);

	free(node);
    }

    free(l);
}
#endif


#ifdef MODULE_n_list_push
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
#endif


#ifdef MODULE_n_list_pop
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
#endif


#ifdef MODULE_n_list_shift
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
#endif


#ifdef MODULE_n_list_unshift
tn_list *n_list_unshift(tn_list *l, void *data)
{
    struct list_node *node;

    if (l->flags & TN_LIST_UNIQ) {
	if (n_list_contains_ex(l, data, NULL))
	    return NULL;
    }
    
    if ((node = malloc(sizeof(*node))) == NULL) {
	return NULL;
    }
    
    node->data = data;
    node->next = l->head;
    l->head = node;

    l->items++;

    return l;
}
#endif


#ifdef MODULE_n_list_remove_nth
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
#endif


#ifdef MODULE_n_list_remove_ex
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
#endif


#ifdef MODULE_n_list_lookup_ex
void *n_list_lookup_ex(const tn_list *l, const void *data, t_fn_cmp cmpf)
{
    register struct list_node *node;


    if (cmpf == NULL)
	cmpf = l->cmp_fn;

    if (cmpf == NULL) {
	trurl_die("n_list_lookup: cmpf function is NULL\n");
	return NULL;
    }
    
    for (node = l->head; node != NULL; node = node->next) {
	if (cmpf(node->data, data) == 0)
	    return node->data;
    }

    return NULL;
}
#endif


#ifdef MODULE_n_list_contains_ex
int n_list_contains_ex(const tn_list *l, const void *data, t_fn_cmp cmpf)
{
    return (n_list_lookup_ex(l, data, cmpf) != NULL);
}
#endif


#ifdef MODULE_n_list_nth
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
#endif


#ifdef MODULE_n_list_iterator
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
#endif


#ifdef MODULE_n_list_size
int n_list_size(const tn_list *l)
{
    return l->items;
}
#endif


#ifdef MODULE_n_list_map_arg
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
#endif

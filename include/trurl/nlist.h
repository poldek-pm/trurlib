/* 
  TRURLib

  $Id$
*/
#ifndef __TRURL_LIST_H
#define __TRURL_LIST_H

#include "tfn_types.h"

typedef struct trurl_list tn_list;

/*#define TN_LLIST_SORTED 0x1*/ /* not implemented */
#define TN_LIST_UNIQ   0x2

tn_list *n_list_new(unsigned int flags,
                    t_fn_free freef,
                    t_fn_cmp  cmpf,
                    t_fn_dup  dupf);

void n_list_free(tn_list *l);


/*
  not implemented yet
  tn_list *n_list_add(tn_list *l, void *data);
*/


/*
  add node to the end of list
 */
tn_list *n_list_push(tn_list *l, void *data);



/*
  remove last node of the list and return it
 */
void *n_list_pop(tn_list *l);



/*
  shift the first node of the list off and return it 
 */
void *n_list_shift(tn_list *l);



/*
  opposite of a n_list_shift()
 */
tn_list *n_list_unshift(tn_list *l, void *data);


/*
  remove nth node of the list and return it
 */
void *n_list_remove_nth(tn_list *l, int nth);


/*
  RET: number of removed nodes
*/
int n_list_remove_ex(tn_list *l, const void *data, t_fn_cmp cmpf);

#define n_list_remove(l, data) n_list_remove_ex(l, data, NULL)


/*
  RET: number of nodes in list
*/
int n_list_size(const tn_list *l);


/*
  RET: nth node 
 */
void *n_list_nth(const tn_list *l, int nth);


/* reset iterator */
void n_list_iterator_start(const tn_list *l);

void *n_list_iterator_get(const tn_list *l);



/*
  Find the node in the list and return it 
 */
void *n_list_lookup_ex(const tn_list *l, const void *data, t_fn_cmp cmpf);

#define n_list_lookup(l, data) n_list_lookup_ex(l, data, NULL)


/*
  RET: bool
*/
int n_list_contains_ex(const tn_list *l, const void *data, t_fn_cmp cmp_f);

#define n_list_contains(l, data) n_list_contains_ex(l, data, NULL)

void *n_list_nth_pop(tn_list *l, unsigned long nth);


/* not finished yet
#define n_list_push_v(l, d) n_list_append((l), (void*)(d))
#define n_list_unshift_v(l, d) n_list_unshift_v((l), (void*)(d))
#define n_list_contains_v(l, d, f) n_list_contains(l, (void*)(d), f)
#define n_list_remove_v(l, d, f) n_list_remove((l), (void*)(d), (f))
#define n_list_lookup_v(l, d, f) n_list_lookup(l, (void*)(d), f);
*/

#endif /* __TRURL_LIST_H */

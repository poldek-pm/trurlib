/* 
  TRURLib
  $Id$
*/

#ifndef TRURL_DIE_H
#define TRURL_DIE_H

#include "tfn_types.h"

typedef void (*t_fn_die_hook) (const char *errmsg);

t_fn_die_hook n_die_set_hook(t_fn_die_hook hook);
void n_die(const char *fmt, ...);

/* internal macro, do not use  */
#define trurl_die__if_frozen(arr) \
    do {                                    \
        if ((arr)->flags & TN_ARRAY_FROZEN) \
            n_die("array is frozen");   \
    } while (0)

#endif

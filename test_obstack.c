/*
   $Id$
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nobstack.h"

int main(int argc, char *argv[])
{
    int n;
    tn_obstack *o;
    o = n_obstack_new(100);
    n = 0;
    while (n < 10000) {
        char *s = o->alloc(o, 100 + n);
        n++;
        if (n % 100 == 0)
            printf("%p %d\n", s, n);

            
    }
    
    return 0;
}

/*
   $Id$
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "nassert.h"
char *n_str_strip_ws(char *str) 
{
    char *p;
    
    if (str == NULL)
        return str;
    
    p = str;
    while(isspace(*p))
        p++;

    str = p;
    p = strchr(str, '\0');
    n_assert(p);
    p--;
    while (p != str && isspace(*p)) {
        *p = '\0';
        p--;
    }
    
    return str;
}

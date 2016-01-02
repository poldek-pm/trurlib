/* 
   TRURLib
   Copyright (C) 1999 Pawel A. Gajda (mis@k2.net.pl)

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "nmalloc.h"
#include "nstr.h"

char *n_str_vconcat(const char *s, va_list ap)
{
    char *p, *rstr;
    va_list save_ap;
    int len;

    len = strlen(s);

    __va_copy(save_ap, ap);

    while ((p = va_arg(ap, char *)) != NULL) {	/* calculate length of args */
        len += strlen(p);
    }

    if ((rstr = n_malloc(len + 1)) == NULL)
        return NULL;

    strcpy(rstr, s);

    __va_copy(ap, save_ap);
    
    while ((p = va_arg(ap, char *)) != NULL) {
        strcat(rstr, p);
    }

    __va_copy(ap, save_ap);
    
    return rstr;
}


char *n_str_concat(const char *s, ...)
{
    char *rstr;
    va_list ap;

    va_start(ap, s);

    rstr = n_str_vconcat(s, ap);

    va_end(ap);

    return rstr;
}

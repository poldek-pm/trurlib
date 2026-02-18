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

#include <string.h>
#include "nassert.h"

char *n_basedirnam(char *path, char **dirname, char **basename)
{
    char *dn, *s = strrchr(path, '/');

    if (s == NULL) {
        dn = NULL;
        if (basename)
            *basename = path;

    } else {
        *s++ = '\0';
        dn  = path;
        if (basename)
            *basename = s;
    }

    if (dirname)
        *dirname = dn;

    return dn;
}

char *n_basenam(const char *path)
{
    char *s = strrchr(path, '/');

    if (s)
        return ++s;
    return (char*)path;
}

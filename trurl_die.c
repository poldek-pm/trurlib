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

/*
   $Id$
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "nassert.h"

typedef void (*t_fn_die_hook) (const char *errmsg);

static t_fn_die_hook die_hook;


t_fn_die_hook set_trurl_err_hook(t_fn_die_hook hook)
{
    t_fn_die_hook tmp = die_hook;
    die_hook = hook;
    return tmp;
}


void trurl_die(const char *fmt,...)
{
    va_list ap;

    va_start(ap, fmt);

    fflush(NULL);

    if (die_hook != NULL) {
	char msg[1024];

	strcpy(msg, "trurlib: ");
	vsprintf(&msg[strlen(msg)], fmt, ap);

	die_hook(msg);

    } else {
	int len;
	fprintf(stderr, "trurlib:");
	vfprintf(stderr, fmt, ap);
	len = strlen(fmt);

	if (len && fmt[len - 1] != '\n')
	    fprintf(stderr, "\n");

	abort();
    }

    va_end(ap);
}

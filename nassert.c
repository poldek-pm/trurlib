/* 
  TRURLib
  Copyright (C) 1999 Pawel Gajda (mis@k2.net.pl)
 
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

#include <stdio.h>
#include <stdlib.h>

#include "nassert.h"

typedef void (*assert_hook_f) (const char *, const char *, int);

static assert_hook_f assert_hook;

assert_hook_f n_assert_sethook(assert_hook_f hook)
{
    assert_hook_f tmp = assert_hook;
    assert_hook = hook;
    return tmp;
}


void n_assert_fail(const char *expr, const char *file, int line)
{
    if (assert_hook != NULL) {
	assert_hook(expr, file, line);

    } else {
	fprintf(stderr, "Assertion %s failed %s:%d\n", expr, file, line);
	abort();
    }
}

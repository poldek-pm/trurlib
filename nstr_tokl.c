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
#include <stdlib.h>
#include <string.h>

#ifdef USE_N_ASSERT
#include "nassert.h"
#else
#include <assert.h>
#define n_assert(expr) assert(expr)
#endif

#ifdef USE_XMALLOCS
#include "xmalloc.h"
#endif

#include "nstr.h"


#define ALLOC_STEP 8
/*
  tokens[0] = s
  tokens[1] = token1
 */
const char **n_str_tokl(const char *s, char *delim)
{
    char **tokens, *scpy;
    char *p;
    int slen, n, tokens_size;

    slen = strlen(s);

    tokens_size = 8;

    if ((tokens = calloc(tokens_size, sizeof(*tokens))) == NULL)
	return NULL;

    if ((tokens[0] = calloc(slen + 1, sizeof(char))) == NULL) {
	free(tokens);
	return NULL;
    }
    scpy = tokens[0];

    n = 1;

    p = (char *) s;

    while (p != NULL && *p) {
	if (n == tokens_size) {
	    char **tmp;
            size_t new_size = (tokens_size + ALLOC_STEP) * sizeof(*tokens);
	    if ((tmp = realloc(tokens, new_size)) == NULL) {
		free(tokens[0]);
		free(tokens);
		tokens = NULL;
		break;

	    } else {
                int i;
                
		tokens = tmp;
		tokens_size += ALLOC_STEP;
                for (i=n; i<tokens_size; i++)
                    tokens[i] = NULL;
	    }
	}
	p = n_str_tok(p, scpy, slen, delim);
	tokens[n++] = scpy;
	scpy += strlen(scpy) + 1;

    }

    if (tokens != NULL)
	tokens++;		/* hide tokens[0] */

    return (const char **) tokens;
}


void n_str_tokl_free(const char **tokens)
{
    if (tokens != NULL)
	tokens--;
    free((char *) tokens[0]);
    free(tokens);
}

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
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nmalloc.h"
#include "narray.h"
#include "nstr.h"


#define ALLOC_STEP 8
/*
  tokens[0] = s
  tokens[1] = token1
 */
const char **n_str_tokl_n(const char *s, const char *delim, int *ntokens)
{
    char **tokens, *scpy;
    char *p;
    int slen, n, tokens_size;

    slen = strlen(s);
    tokens_size = 8;

    if ((tokens = n_calloc(tokens_size, sizeof(*tokens))) == NULL)
        return NULL;

    if ((tokens[0] = n_calloc(slen + 1, sizeof(char))) == NULL) {
        free(tokens);
        return NULL;
    }
    	
    scpy = tokens[0];
    n = 1;
    p = (char *) s;

    while (p != NULL && *p) {
        p = n_str_tok(p, scpy, slen, delim);
        tokens[n++] = scpy;
        scpy += strlen(scpy) + 1;

        if (n == tokens_size) {
            char **tmp;
            size_t new_size = (tokens_size + ALLOC_STEP) * sizeof(*tokens);
            if ((tmp = n_realloc(tokens, new_size)) == NULL) {
                free(tokens[0]);
                free(tokens);
                tokens = NULL;
                break;

            } else {
                int i;
                
                tokens = tmp;
                tokens_size += ALLOC_STEP;
                for (i=n; i < tokens_size; i++)
                    tokens[i] = NULL;
            }
        }
    }
    if (tokens != NULL)
        tokens++;		/* hide tokens[0] */

    if (ntokens) {
        if (tokens)
            *ntokens = n;
        else
            ntokens = 0;
    }

    return (const char **) tokens;
}

#undef n_str_tokl
const char **n_str_tokl(const char *s, const char *delim) 
{
    return n_str_tokl_n(s, delim, NULL);
}

void n_str_tokl_free(const char **tokens)
{
    if (tokens != NULL)
	tokens--;
    free((char *) tokens[0]);
    free(tokens);
}



#define ST_WHITE 0
#define ST_TOKEN 1
#define ST_QUOTE 2
#define ST_OZONE 3

struct lp_state {
    const char *line;
    int        lindex;

    const char *white;
    const char *brk;
    const char *quote;
    char       escape;
    
    int       state;      
    char      curr_quote; 
    char      is_break;
    int       is_quoted;
};

static inline
int cindex(char c, const char *s)
{
    char *p;

    if ((p = strchr(s, c)))
        return (int)(p - s);
    return -1;
}

static inline
void storechr(char *s, int slen, int *sindex, char c)
{

    if (*sindex >= 0 && *sindex < slen) {
        s[*sindex] = c;
        (*sindex)++;
    }
}

static 
void lp_init(struct lp_state *st, 
             const char *line, const char *white, const char *brk,
             const char *quote, char escape) 
{
    memset(st, 0, sizeof(*st));
    st->line = line;
    st->white = white;
    st->brk = brk;
    st->quote = quote;
    st->escape = escape;
    
}


static
int lp_parse(struct lp_state *st, char *token, int toksize, int *toklen)

{
    int tokindex;
    char c;

    tokindex = 0;
    *token = '\0';
    
    st->state = ST_WHITE;       /* initialize state */
    st->curr_quote = 0;           /* initialize previous quote char */
    st->is_break = 0;
    st->is_quoted = 0;
  
    if((c = st->line[st->lindex]) == '\0')
        return 0;

    if (toklen)
        *toklen = 0;

    while (c) {
        char *p;

        if ((p = strchr(st->brk, c))) {
            switch(st->state) {
                case ST_WHITE:
                case ST_TOKEN:
                    st->lindex++;
                    st->is_break = *p;
                    goto l_end;
        
                case ST_QUOTE:
                    storechr(token, toksize, &tokindex, c);
                    break;
            }
          
        } else if ((p = strchr(st->quote, c))) {
            switch(st->state) {
                case ST_WHITE: 
                    st->state = ST_QUOTE;
                    st->curr_quote = *p;
                    st->is_quoted = 1;	
                    break;
  
                case ST_QUOTE:
                    if (*p != st->curr_quote)
                        storechr(token, toksize, &tokindex, c);
                    else {
                        st->state = ST_WHITE;
                        st->curr_quote = 0;
                    }
                    break;

                case ST_TOKEN:
                    st->state = ST_WHITE;
                    goto l_end;
            }
          
        } else if ((p = strchr(st->white, c))) { 
            switch(st->state) {
                case ST_WHITE:
                    break;		/* keep going */
                    
                case ST_TOKEN:
                    st->state = ST_WHITE;
                    goto l_end;
                    break;
          
                case ST_QUOTE:
                    storechr(token, toksize, &tokindex, c);
                    break;
            }
          
        } else if (c == st->escape) {
            char nc;
            
            nc = st->line[st->lindex + 1];
            if (nc == 0 || nc == st->escape) {			/* EOF */
                st->is_break = 0;
                storechr(token, toksize, &tokindex, c);
                st->lindex++;
                if (nc == 0)
                    goto l_end;
            }
          
            switch(st->state) {
                case ST_WHITE:
                    st->lindex--;
                    st->state = ST_TOKEN;
                    break;
                  
                case ST_TOKEN:
                case ST_QUOTE:
                    storechr(token, toksize, &tokindex, st->line[++st->lindex]);
                    break;
            }
          
        } else  {
            switch(st->state) {
                case ST_WHITE:
                    st->state = ST_TOKEN;
                  
                case ST_TOKEN:
                case ST_QUOTE:
                    storechr(token, toksize, &tokindex, c);
                    break;
            }
        }

        c = st->line[++st->lindex];
    }
  

 l_end:
//    printf("%d, %d\n", tokindex, toksize);
    n_assert(tokindex < toksize);
    token[tokindex] = '\0';
    if (toklen)
        *toklen = tokindex;
    return 1;
}


tn_array *n_str_etokl_ext(const char *line, const char *white,
                          const char *brk, const char *quote,
                          char escape) 
{
    const char       *default_white = " \t", 
        *default_brk   = ";|", 
        *default_quote = "\"'";
    char             default_escape = '\\';
    char             *token;
    tn_array         *tl;
    struct lp_state  st;
    int              toklen, toksize;
    
    if (white == NULL)
        white = default_white;
    
    if (brk == NULL)
        brk = default_brk;
    
    if (quote == NULL)
        quote = default_quote;
    
    if (escape == '\0')
        escape = default_escape;


   lp_init(&st, line, white, brk, quote, escape);
   toksize = strlen(line) + 1;
   token = alloca(toksize + 1);

   tl = n_array_new(4, free, (tn_fn_cmp)strcmp);

   while (lp_parse(&st, token, toksize, &toklen) > 0) {
       
       if (toklen > 0)
           n_array_push(tl, n_strdupl(token, toklen));
       
       if (st.is_break) {
           char break_str[2];

           break_str[0] = st.is_break;
           break_str[1] = '\0';
           n_array_push(tl, n_strdup(break_str));
       }
   }
   
   return tl;
}

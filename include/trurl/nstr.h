/* 
  TRURLib

  $Id$
*/

#ifndef __TRURL_STR_H
#define __TRURL_STR_H


char *n_str_tok(const char *s, char *tok, size_t toklen, char *brk);

/*
  Accepts a string and breaks it into words.

  RET: NULL terminated array of tokens. The array is allocated dynamically
       by this function, and you *must* release it by  n_str_tokl_free().
       Returns NULL if there was insufficient memory.
*/
const char **n_str_tokl(const char *s, char *delim);
void n_str_tokl_free(const char **tokens);



#include <stdarg.h>

/*
  Calculate length of strings 
  RET: length
*/
int n_str_vlen(const char *s, va_list ap);
int n_str_len(const char *s, ...);


/*
  Concatenate strings
       
  RET: New string. The string is allocated dynamically
       by this function, and you *must* release it by free()
       Returns NULL if there was insufficient memory.
*/
char *n_str_concat(const char *s, ...);
char *n_str_vconcat(const char *s, va_list ap);

#endif /* __TRURL_STR_H */

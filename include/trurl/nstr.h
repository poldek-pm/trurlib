/* 
  TRURLib

  $Id$
*/

#ifndef TRURL_STR_H
#define TRURL_STR_H

#include <stdarg.h>

#ifndef __va_copy
# define __va_copy(a, b) ((a) = (b))
#endif

char *n_str_tok(const char *s, char *tok, size_t toklen, char *brk);

/*
  Accepts a string and breaks it into words.

  RET: NULL terminated array of tokens. The array is allocated dynamically
       by this function, and you *must* release it by n_str_tokl_free().
       Returns NULL if there was insufficient memory.
*/
const char **n_str_tokl_n(const char *s, char *delim, int *ntokens);
#define n_str_tokl(s, delim) n_str_tokl_n(s, delim, NULL)
void n_str_tokl_free(const char **tokens);

#include <trurl/narray.h>
tn_array *n_str_etokl_ext(const char *line, const char *white,
                          const char *brk, const char *quote,
                          char escape);

#define n_str_etokl(line) n_str_etokl_ext(line, NULL, NULL, NULL, 0)


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


/*
  Similar to strncpy(3), but:
  - always terminate dest
  - returns pointer to last dest '\0'
*/
char *n_strncpy(char *dest, const char *src, size_t n);



/* WARN: it modifies path! */
int n_basedirnam(char *path, char **dirname, char **basename);
char *n_basenam(const char *path);

#endif /* TRURL_STR_H */

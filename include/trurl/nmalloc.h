/* 
  TRURLib

  $Id$
*/
#ifndef TRURL_NMALLOC_H
#define TRURL_NMALLOC_H

#include <stddef.h>
#include <stdlib.h>

void (*n_malloc_set_failhook(void (*fn) (void)));
void *n_malloc(size_t size);
void *n_calloc(size_t nmemb, size_t size);
void *n_realloc(void *ptr, size_t size);
char *n_strdup(const char *s);
void n_free(void *ptr);
void n_cfree(void *ptr);

char *n_strdupl(const char *s, size_t length);
void *n_memdup(const void *ptr, size_t size);

/* n_strdupap(src, &dest) */
#define n_strdupap(s, dp)                     \
   do {	                                      \
      const char *ss = (s);                   \
      char  **dptr = (dp);                    \
      int len = strlen(ss) + 1;               \
      *dptr = alloca(len);	                  \
      memcpy(*dptr, ss, len);				  \
   } while (0);

#endif /* NMALLOC_H */

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

char *n_strdupl(const char *s, size_t length);

void *n_memdup(const void *ptr, size_t size);

#endif /* NMALLOC_H */

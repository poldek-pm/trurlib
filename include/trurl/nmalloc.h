/* 
  TRURLib

  $Id$
*/
#ifndef TRURL_NMALLOC_H
#define TRURL_NMALLOC_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

void (*n_malloc_set_failhook(void (*fn) (void)));
void *n_malloc(size_t size);
void *n_calloc(size_t nmemb, size_t size);
void *n_realloc(void *ptr, size_t size);
char *n_strdup(const char *s);
void n_free(void *ptr);
void n_cfree(void *ptr);

char *n_strdupl(const char *s, size_t length);
void *n_memdup(const void *ptr, size_t size);


/* call it n_strdupap(src, &dest) */
#define n_strdupap(s, dp)                     \
   do {	                                      \
      const char *ss = (s);                   \
      char  **dptr = (dp);                    \
      int len = strlen(ss) + 1;               \
      *dptr = alloca(len);	                  \
      memcpy(*dptr, ss, len);				  \
   } while (0);




struct trurl_alloc_private {
    uint16_t    _refcnt;
    uint16_t    _flags;
    void*       (*na_malloc)(struct trurl_alloc_private *, size_t size);
    void*       (*na_calloc)(struct trurl_alloc_private *, size_t size);
    void*       (*na_realloc)(struct trurl_alloc_private *, void *ptr,
                              size_t size, size_t newsize);
    void        (*na_free)(struct trurl_alloc_private *, void *);
    void        *_privdata;
};

typedef struct trurl_alloc_private tn_alloc;

#define TN_ALLOC_MALLOC  (1 << 0)
#define TN_ALLOC_OBSTACK (1 << 1)

tn_alloc *n_alloc_new(size_t chunkkb, unsigned int flags);
void n_alloc_free(tn_alloc *na);


#endif /* NMALLOC_H */


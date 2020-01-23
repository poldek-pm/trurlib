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


/* strdup && alloca; call it n_strdupap(src, &dest) */
#define n_strdupap(s, dp)                     \
   do {	                                      \
      const char *ss = (s);                   \
      char  **dptr = (dp);                    \
      int len = strlen(ss) + 1;               \
      *dptr = alloca(len);                    \
      memcpy(*dptr, ss, len);        	      \
   } while (0);

#define n_strdupapl(s, length,  dp)           \
   do {	                                      \
      const char *ss = (s);                   \
      char  **dptr = (dp);                    \
      *dptr = alloca(length + 1);             \
      memcpy(*dptr, ss, length + 1);          \
   } while (0);

/* obstack allocator */
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


/* string deduplication allocator */
struct trurl_strdalloc_private;
typedef struct trurl_strdalloc_private tn_strdalloc;

tn_strdalloc *n_strdalloc_new(size_t initial_slots, int flags);
void n_strdalloc_free(tn_strdalloc *sa);

struct trurl_lstr8_private {
    uint8_t len;
    char str[];
};
typedef struct trurl_lstr8_private tn_lstr8;

const tn_lstr8 *n_strdalloc_add8(tn_strdalloc *sa, const char *str, size_t len);


struct trurl_lstr16_private {
    uint16_t len;
    char str[];
};
typedef struct trurl_lstr16_private tn_lstr16;

const tn_lstr16 *n_strdalloc_add16(tn_strdalloc *sa, const char *str, size_t len);


#endif /* NMALLOC_H */

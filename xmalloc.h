/* 
  TRURLib

  $Id$
*/
#ifndef __XMALLOCS_H
#define __XMALLOCS_H

void (*setxmallocs_handler( void(*handler)(void) )) (void);
#ifdef USE_XMALLOCS

#include <stddef.h>

#undef xmalloc
#undef xcalloc
#undef xrealloc
#undef xstrdup
#undef xfree
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);
void xfree(void *ptr);

     
#ifdef malloc 
# error "malloc already defined!"
# undef malloc
#endif 

#ifdef calloc 
# error "calloc already defined!"
# undef calloc
#endif 

#ifdef realloc
# error "realloc already defined!"
# undef realloc
#endif 

#ifdef strdup 
# error "strdup already defined!"
# undef strdup
#endif 

#ifdef free 
# error "free already defined!"
# undef free
#endif 

#define malloc(a)      xmalloc((a))
#define calloc(a, b)   xcalloc((a), (b))
#define realloc(a, b)  xrealloc((a), (b))
#define strdup(a)      xstrdup((a))
#define free(a)        xfree((a))

#endif /* USE_XMALLOCS */

#endif /* __XMALLOCS_H */

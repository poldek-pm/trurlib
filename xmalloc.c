#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "nmalloc.h"
/* be compatibile with < 0.43.7 */

void *xmalloc(size_t size)
{
    return n_malloc(size);
}

void *xcalloc(size_t nmemb, size_t size)
{
    return n_calloc(nmemb, size);
}

void *xrealloc(void *ptr, size_t size)
{
    return n_realloc(ptr, size);
}

char *xstrdup(const char *s) 
{
    return n_strdup(s);
}

void xfree(void *ptr) 
{
    n_free(ptr);
}


/* $Id$ */

#ifndef TRURL_N_SNPRINTF_H
#define TRURL_N_SNPRINTF_H

#include <stdarg.h>
#include <stdio.h>

/*
  n_*snprintf() always returns the number of characters printed
*/

static inline
int n_vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
    int s;

    s = vsnprintf(str, size, fmt, ap);
    if (s < 0 || (unsigned)s > size)
        return size - 1;
    
    return s;
}

static inline
int n_snprintf(char *str, size_t size, const char *fmt, ...)
{
    va_list ap;
    int s;
    
    va_start(ap, fmt);
    s = n_vsnprintf(str, size, fmt, ap);
    va_end(ap);
    
    return s;
}

#endif

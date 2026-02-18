#ifndef TRURL_NASSERT_H
#define TRURL_NASSERT_H

#ifdef n_assert
#error "n_assert already defined"
#endif

void (*n_assert_set_hook( void(*hook)(const char*, const char*, int) ) )
     (const char*, const char*, int);

void n_assert_fail(const char* expr, const char* file, int line);

#define n_assert(expr)	\
   ((void) ((expr) ? 0 : (n_assert_fail(#expr, __FILE__, __LINE__), 0)))

#endif

/*
   $Id$
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "nassert.h"
#include "nmalloc.h"


void assert_hook(const char *expr, const char *file, int line)
{
    printf("n_assert hook: Ajajaj! %s, %s:%d -- OK\n", expr, file, line);
    /*exit(1); */
}

void trurldie_hook(const char *expr, const char *file, int line)
{
    printf("\nn_assert hook: Ajajaj! %s, %s:%d -- OK\n", expr, file, line);
    /*exit(1); */
}

void xmallocs_handler(void)
{
    printf("\nxmallocs_test: xmallocs_handler: memory full -- OK\n");
    /*exit(1); */
}

int main()
{

    void *p;

    printf("n_assert test:");
    n_assert_sethook(assert_hook);
    n_assert(0);
    n_assert_sethook(NULL);


    setxmallocs_handler(xmallocs_handler);
    p = n_malloc(1024 * 1024 * 1024);

    return 0;
}

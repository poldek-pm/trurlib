/*
   $Id$
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "nassert.h"
#include "nmalloc.h"
#include "nbuf.h"


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

void test_buf(void)
{
    tn_buf *nbuf;

    nbuf = n_buf_new(64);
    n_buf_printf(nbuf, "dupa blada i co z tego\n");
    n_buf_seek(nbuf, 2, SEEK_SET);
    n_buf_printf(nbuf, "zo");
    n_buf_seek(nbuf, 0, SEEK_END);
    n_buf_puts_z(nbuf, "AAAA\n");
    printf("%d, %d (%s)\n", n_buf_size(nbuf), strlen(n_buf_ptr(nbuf)),
           n_buf_ptr(nbuf));
}


int main()
{

    void *p;

    test_buf();
    
    printf("n_assert test:");
    n_assert_sethook(assert_hook);
    n_assert(0);
    n_assert_sethook(NULL);


    n_malloc_set_failhook(xmallocs_handler);
    p = n_malloc(1024 * 1024 * 1024);

    return 0;
}

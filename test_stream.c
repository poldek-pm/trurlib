/*
   $Id$
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nstream.h"
#include "nstore.h"
#include "nassert.h"
#include "nmalloc.h"


void test_write(const char *name, const char *mode)
{
    tn_stream *st;

    if (mode == NULL) mode = "w";
    
    st = n_stream_open(name, mode, TN_STREAM_UNKNOWN);
    n_stream_printf(st, "dupa blada\n i co z tego\n");
    n_stream_seek(st, 0, SEEK_SET);
    n_stream_printf(st, "ALA");
    n_stream_close(st);
}

void test_read(const char *name)
{
    tn_stream *st;
    char buf[1024] = { '\0' };
    
    st = n_stream_open(name, "r", TN_STREAM_UNKNOWN);
    n_stream_read(st, buf, sizeof(buf));
    printf("read %s\n", buf);
    n_stream_close(st);
}

void test_getline(const char *name)
{
    tn_stream *st;
    char *buf = n_malloc(16);
    int n;
    
    st = n_stream_open(name, "r", TN_STREAM_UNKNOWN);
    n = n_stream_getline(st, &buf, 16);
    printf("getline = %d (%s)\n", n, buf);
    n_assert(strlen(buf) == n);
    n_stream_close(st);
}



int main(int argc, char *argv[])
{
    if (argc > 1) {
        //test_write(argv[1], "a+");
        test_getline(argv[1]);
    }
    
    return 0;
}

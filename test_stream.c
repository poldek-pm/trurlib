/*
   $Id$
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nstream.h"
#include "nstore.h"

void test_write(const char *name)
{
    tn_stream *st;

    st = n_stream_open(name, "w", TN_STREAM_UNKNOWN);
    n_stream_printf(st, "dupa blada\n i co z tego\n");
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


int main(int argc, char *argv[])
{
    if (argc > 1) {
        test_write(argv[1]);
        test_read(argv[1]);
    }
    
    return 0;
}

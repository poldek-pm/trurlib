#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "nassert.h"
#include "nmalloc.h"
#include "nbuf.h"

#include "n_snprintf.h"
#include "n_check.h"
#include "nbuf.h"
#include "ndie.h"



START_TEST(test_buf)
{
    tn_buf *nbuf;

    nbuf = n_buf_new(64);
    n_buf_printf(nbuf, "%s", "foo bar baz\n");
    n_buf_seek(nbuf, 0, SEEK_END);
    n_buf_puts_z(nbuf, "FOO\n");
    expect_str(n_buf_ptr(nbuf), "foo bar baz\nFOO\n");

    n_buf_free(nbuf);
}
END_TEST

START_TEST(test_buf_iterator)
{
    tn_buf *nbuf;
    tn_buf_it it;
    char *line;
    size_t len;

    nbuf = n_buf_new(4);
    n_buf_printf(nbuf, "%s", "foo\nbar\nbaz");

    n_buf_it_init(&it, nbuf);

    line = n_buf_it_gets(&it, &len);
    expect_int(strncmp(line, "foo", 3), 0);

    line = n_buf_it_gets(&it, &len);
    expect_int(strncmp(line, "bar", 3), 0);

    line = n_buf_it_gets(&it, &len);
    expect_int(strncmp(line, "baz", 3), 0);

    line = n_buf_it_gets(&it, &len);
    expect_null(line);

    n_buf_free(nbuf);
}
END_TEST

NTEST_RUNNER("nbuf", test_buf, test_buf_iterator);

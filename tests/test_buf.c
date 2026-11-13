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

/* size within the block: without ASan a write past it can pass every
   other check */
#define expect_in_block(b) ck_assert_uint_le((b)->size, (b)->allocated)

/* as poldek's pkgfl_store(): room for a size, filled in after the data */
START_TEST(test_buf_seek_past_end)
{
    tn_buf *nbuf = n_buf_new(16);

    n_buf_write(nbuf, "0123456789abc", 13); /* 3 bytes left in the block */
    n_buf_seek(nbuf, 4, SEEK_CUR);
    n_buf_write(nbuf, "data", 4);
    expect_in_block(nbuf);
    expect_int(n_buf_size(nbuf), 21);

    n_buf_seek(nbuf, 13, SEEK_SET);
    n_buf_write(nbuf, "SIZE", 4);
    expect_int(memcmp(n_buf_ptr(nbuf), "0123456789abcSIZEdata", 21), 0);

    n_buf_seek(nbuf, 100, SEEK_SET);    /* far past the block */
    n_buf_write(nbuf, "end", 3);
    expect_in_block(nbuf);
    expect_int(n_buf_size(nbuf), 103);

    n_buf_free(nbuf);
}
END_TEST

START_TEST(test_buf_clean)
{
    char data[] = "caller's data";
    tn_buf *nbuf = n_buf_new(0);

    n_buf_clean(nbuf);                  /* nothing written yet */
    expect_int(n_buf_size(nbuf), 0);

    n_buf_init(nbuf, data, sizeof(data));
    n_buf_clean(nbuf);                  /* lets the caller's data go */
    expect_int(n_buf_size(nbuf), 0);
    n_buf_puts(nbuf, "ab");
    expect_in_block(nbuf);
    expect_int(n_buf_size(nbuf), 2);

    n_buf_free(nbuf);
}
END_TEST

START_TEST(test_buf_printf_long)
{
    /* with ".", one byte more than n_buf_vprintf()'s stack buffer holds */
    char s[4096];
    tn_buf *nbuf = n_buf_new(64);

    memset(s, 'x', sizeof(s) - 1);
    s[sizeof(s) - 1] = '\0';
    expect_int(n_buf_printf(nbuf, "%s.", s), 4096);
    expect_int(n_buf_size(nbuf), 4096);
    expect_int(((char *) n_buf_ptr(nbuf))[4095], '.');
    expect_int(((char *) n_buf_ptr(nbuf))[4096], '\0');

    n_buf_free(nbuf);
}
END_TEST

START_TEST(test_buf_printf_format_error)
{
    tn_buf *nbuf = n_buf_new(64);

    /* the C locale has no multibyte form of U+0100 */
    expect_int(n_buf_printf(nbuf, "%ls", L"\x100"), -1);
    expect_int(n_buf_size(nbuf), 0);

    n_buf_free(nbuf);
}
END_TEST

static int ndies = 0;
static void die_hook(const char *msg)
{
    (void) msg;
    ndies++;
}

START_TEST(test_buf_printf_no_room)
{
    char data[] = "abc";
    tn_buf *nbuf = n_buf_new(0);

    n_die_set_hook(die_hook);
    n_buf_init(nbuf, data, sizeof(data)); /* const size, full */
    n_buf_seek(nbuf, 0, SEEK_END);
    expect_int(n_buf_printf(nbuf, "%s", "de"), -1);
    expect_int(ndies, 1);
    expect_int(n_buf_size(nbuf), 4);
    expect_int(n_buf_tell(nbuf), 4);
    expect_int(memcmp(data, "abc", 4), 0);

    n_buf_clean(nbuf);                  /* a buffer of its own now */
    expect_int(n_buf_printf(nbuf, "%s", "de"), 2);
    expect_str(n_buf_ptr(nbuf), "de");

    n_buf_free(nbuf);
}
END_TEST

NTEST_RUNNER("nbuf", test_buf, test_buf_iterator, test_buf_seek_past_end,
             test_buf_clean, test_buf_printf_long,
             test_buf_printf_format_error, test_buf_printf_no_room);

#include "n_check.h"
#include "nmalloc.h"
#include "nstream.h"
#include "nstore.h"

void do_test_read_write(const char *tmppath)
{
    char buf[75];
    int i, nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;

    tn_stream *io = n_stream_open(tmppath, "w", TN_STREAM_UNKNOWN);
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        nw += n_stream_write(io, buf, n);
    }
    n_stream_close(io);

    io = n_stream_open(tmppath, "r", TN_STREAM_UNKNOWN);
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        char rbuf[75];

        memset(rbuf, 0, sizeof(rbuf));

        int x = n_stream_read(io, rbuf, n);
        expect_int(x, n);
        expect_int(memcmp(buf, rbuf, n), 0);
        nr += x;
        //printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);

    }
    n_stream_close(io);
    expect_int(nw, nr);
    //printf("OK\n");
}

START_TEST(test_read_write)
{
    do_test_read_write(NTEST_TMPPATH("nstream-rw.txt"));
    do_test_read_write(NTEST_TMPPATH("nstream-rw.gz"));
    do_test_read_write(NTEST_TMPPATH("nstream-rw.zst"));
}
END_TEST

static void seek_and_read(tn_stream *io, int nth)
{
    char buf[75], rbuf[75];
    char *fmt = "line%.12d";
    int n, x;


    n_stream_seek(io, nth * 16, SEEK_SET);
    expect_int(n_stream_tell(io), nth * 16);

    n = snprintf(buf, sizeof(buf), fmt, nth);
    x = n_stream_read(io, rbuf, n);

    expect_int(x, n);
    expect_int(memcmp(buf, rbuf, n), 0);
}

void do_test_seek(const char *tmppath)
{
    char buf[75];
    char *fmt = "line%.12d";
    tn_stream *io;
    //char rbuf[75];
    int i;

    io = n_stream_open(tmppath, "w", TN_STREAM_UNKNOWN);
    for (i = 0; i < 100; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        int nn = n_stream_write(io, buf, n);
        expect_int(nn, n);
    }
    n_stream_close(io);

    io = n_stream_open(tmppath, "r", TN_STREAM_UNKNOWN);
    expect_int(n_stream_tell(io), 0);

    seek_and_read(io, 1);
    expect_int(n_stream_tell(io), 32);

    seek_and_read(io, 0);
    expect_int(n_stream_tell(io), 16);

    seek_and_read(io, 1);
    expect_int(n_stream_tell(io), 32);

    seek_and_read(io, 2);
    expect_int(n_stream_tell(io), 48);
}

START_TEST(test_seek) {
    do_test_seek(NTEST_TMPPATH("nstream-seek.txt"));
    do_test_seek(NTEST_TMPPATH("nstream-seek.gz"));
    do_test_seek(NTEST_TMPPATH("nstream-seek.zst"));
}
END_TEST


/* n_stream_getline() returns one line at a time and grows its buffer when
   the line does not fit; a line that ends exactly at the buffer boundary
   must not pull in the next one. Run over a plain, a gzip and a zstd
   stream, since st_gets and st_getc are separate wrappers there. */
static void do_test_getline(const char *path, size_t startsize)
{
    static const char *lines[] = {
        "short\n",
        "12345678901234\n",     /* 15 bytes, exactly the buffer size - 1 */
        "a longer line that does not fit and grows the buffer: "
        "0123456789 0123456789\n",
        "last line without a newline",
        NULL
    };
    tn_stream *st;
    char *buf;
    size_t size;
    int i;

    st = n_stream_open(path, "w", TN_STREAM_UNKNOWN);
    fail_if(st == NULL, "cannot create %s", path);
    for (i = 0; lines[i] != NULL; i++)
        n_stream_write(st, lines[i], strlen(lines[i]));
    n_stream_close(st);

    st = n_stream_open(path, "r", TN_STREAM_UNKNOWN);
    fail_if(st == NULL, "cannot open %s", path);

    size = startsize;
    buf = startsize ? n_malloc(startsize) : NULL;
    for (i = 0; lines[i] != NULL; i++) {
        int n = n_stream_getline(st, &buf, &size);

        expect_int(n, strlen(lines[i]));
        expect_str(buf, lines[i]);
    }
    expect_int(n_stream_getline(st, &buf, &size), 0);     /* EOF */

    /* startsize 0 means *bufptr = NULL: the buffer is allocated as
       MAX_CANON right away, so only an explicit buffer can grow here */
    if (startsize > 0)
        fail_unless(size > startsize,
                    "the grown buffer size was not reported: %zu", size);

    n_free(buf);
    n_stream_close(st);
}

START_TEST(test_stream_getline)
{
    do_test_getline(NTEST_TMPPATH("nstream-getline.txt"), 16);
    do_test_getline(NTEST_TMPPATH("nstream-getline.gz"), 16);
    do_test_getline(NTEST_TMPPATH("nstream-getline.zst"), 16);
    do_test_getline(NTEST_TMPPATH("nstream-getline-alloc.txt"), 0);
}
END_TEST

START_TEST(test_open) {
    tn_stream *st = n_stream_open("/tmp/non_existing_file_trurlib_nstream.txt.zst", "r", TN_STREAM_UNKNOWN);
    n_assert(st == NULL);
}
END_TEST


NTEST_RUNNER("nstream",
            test_open,
            test_read_write,
            test_seek,
            test_stream_getline);

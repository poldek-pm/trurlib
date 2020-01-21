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
    int nw = 0;
    tn_stream *io;
    //char rbuf[75];
    int i;

    io = n_stream_open(tmppath, "w", TN_STREAM_UNKNOWN);
    for (i = 0; i < 100; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        int nn = n_stream_write(io, buf, n);
        expect_int(nn, n);
        nw += n;
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


void test_getline(const char *name)
{
    tn_stream *st;
    char *buf = n_malloc(16);
    int n;

    st = n_stream_open(name, "r", TN_STREAM_UNKNOWN);
    n = n_stream_getline(st, &buf, 16);
    printf("getline = %d (%s)\n", n, buf);
    n_assert((int)strlen(buf) == n);
    n_stream_close(st);
}

NTEST_RUNNER("nstream",
            test_read_write,
            test_seek);
